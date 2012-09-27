/*
 *  BermudaOS - TWI interface
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/twif.h>

#include <arch/avr/io.h>
#include <arch/avr/twif.h>
#include <arch/avr/328/dev/twibus.h>
#include <arch/avr/interrupts.h>

/**
 * \file arch/avr/328/dev/twibus.c Hardware TWI bus controller.
 */

#ifdef __EVENTS__
/**
 * \var twi0_mutex
 * \brief Bus mutex.
 * 
 * Priority queue for bus 0.
 */
static volatile void *twi0_mutex = SIGNALED;

/**
 * \var twi0_master_queue
 * \brief Master transfer queue.
 * 
 * Threads are placed in this queue when they are waiting for a TW master transfer to
 * complete.
 */
static volatile void *twi0_master_queue = SIGNALED;

/**
 * \var twi0_slave_queue
 * \brief Slave transfer queue.
 * 
 * Threads are placed in this queue when they are waiting for a TW slave transfer to
 * complete.
 */
static volatile void *twi0_slave_queue = SIGNALED;
#endif

/**
 * \var twibus0
 * \brief Global pointer to the TWI0 bus.
 * \see TWI0
 */
TWIBUS *twibus0 = NULL;

static HWTWI twi0hw = {
	&TWBR,
	&TWCR,
	&TWSR,
	&TWDR,
	&TWAR,
	&TWAMR,
	.scl = 5,
	.sda = 4,
};

/**
 * \brief Initialize TWI bus 0.
 * \param sla Own slave address.
 */
PUBLIC void BermudaTwi0Init(unsigned char sla)
{
	TWIBUS *bus;
	if(TWI0 != NULL) { // already initialized
		return;
	}
	if((bus = BermudaTwiBusFactoryCreate(sla)) == NULL) {
		return;
	}

	TWI0 = bus;
	
#ifdef __EVENTS__
	bus->mutex = &twi0_mutex;
	bus->master_queue = &twi0_master_queue;
	bus->slave_queue = &twi0_slave_queue;
#else
	bus->mutex = 0;
	bus->master_queue = 1;
	bus->slave_queue = 1;
#endif

	// Initialize the hardware interface.
	bus->io.hwio = (void*)&twi0hw;
	BermudaTwiIoData(bus)->io_in = AvrIO->pinc;
	BermudaTwiIoData(bus)->io_out = AvrIO->portc;
}

/**
 * \brief Initializes the TWI structes.
 * \param sla Slave address to set.
 * \note The TWIBUS, TWIF and HWTWI will be initialized by this function.
 *
 * All data structures needed to use the hardware TW interface will be initialized
 * by this function.
 */
PUBLIC TWIBUS *BermudaTwiBusFactoryCreate(unsigned char sla)
{
	TWIBUS *bus;
	TWIF   *twif;
	
	bus = BermudaHeapAlloc(sizeof(*bus));
	twif = BermudaHeapAlloc(sizeof(*twif));

	if(!bus && !twif) {
		return NULL;
	}

	// Initialize the TW interface.
	bus->twif = twif;
	bus->twif->transfer = &BermudaAvrTwMasterTransfer;
	bus->twif->io = &BermudaAvrTwIoctl;
	bus->twif->ifbusy = &BermudaAvrTwHwIfacBusy;
	bus->twif->listen = &BermudaAvrTwSlaveListen;
	bus->twif->respond = &BermudaAvrTwSlaveRespond;
	BermudaAvrTwIrqAttatch(bus, &BermudaAvrTwISR);

	// Initialize other parts of the bus
	bus->busy = false;
	bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
	bus->twif->io(bus, TW_SET_SLA, &sla);
	bus->twif->io(bus, TW_SET_GCR, NULL);

	return bus;
}

/**
 * \brief Calculate the value of the TWBR register.
 * \param freq Wanted frequency.
 * \param pres Used prescaler.
 * \note The <b>pres</b> parameter can have one of the following values: \n
 *       * TW_PRES_1 \n
 *       * TW_PRES_4 \n
 *       * TW_PRES_16 \n
 *       * TW_PRES_64
 * \see TW_PRES_1
 * \see TW_PRES_4 
 * \see TW_PRES_16 
 * \see TW_PRES_64
 * 
 * The needed value of the TWBR register will be calculated using the given
 * (and used) prescaler value.
 */
PUBLIC unsigned char BermudaTwiCalcTWBR(uint32_t freq, unsigned char pres)
{
	char prescaler;
	uint32_t twbr;
	
	switch(pres) {
		case TW_PRES_1:
			prescaler = 1;
			break;
		case TW_PRES_4:
			prescaler = 4;
			break;
		case TW_PRES_16:
			prescaler = 16;
			break;
		case TW_PRES_64:
			prescaler = 64;
			break;
		default:
			prescaler = -1;
			break;
	}
	
	if(prescaler == -1) {
		return 0xFF;
	}
	
	twbr = (F_CPU - (16*freq)) / (2*prescaler*freq);
	
	return twbr & 0xFF;
}

/**
 * \brief Calculates the TWI prescaler value.
 * \param frq The desired frequency.
 * \return The prescaler value in hardware format: \n
 *         * B0 for a prescaler of 1; \n
 *         * B1 for a prescaler of 4; \n
 *         * B10 for a prescaler of 16; \n
 *         * B11 for a prescaler of 64
 *
 * Calculates the prescaler value based on the given frequency.
 */
PUBLIC unsigned char BermudaTwiCalcPres(uint32_t frq)
{
	unsigned char ret = 0;
	
	if(frq > TWI_FRQ(255,1)) {
		ret = B0;
	}
	else if(frq > TWI_FRQ(255,4) && frq < TWI_FRQ(1,4)) {
		ret = B11;
	}
	else if(frq > TWI_FRQ(255,16) && frq < TWI_FRQ(1,16)) {
		ret = B10;
	}
	else if(frq > TWI_FRQ(255,64) && frq < TWI_FRQ(1,64)) {
		ret = B1;
	}

	return ret;
}

SIGNAL(TWI_STC_vect)
{
	twibus0->twif->isr(twibus0);
}

