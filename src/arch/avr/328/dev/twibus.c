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

#if defined(__TWI__) || defined(__DOXYGEN__)

/**
 * \file arch/avr/328/dev/twibus.c Hardware TWI bus controller.
 */

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/twif.h>

#include <arch/avr/io.h>
#include <arch/avr/328/dev/twibus.h>

#include <arch/avr/interrupts.h>

// private functions
PRIVATE WEAK int BermudaTwHwIfacBusy(TWIBUS *bus);

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

static TWIHW twi0hw = {
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

	bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
	bus->twif->io(bus, TW_SET_SLA, &sla);
	bus->twif->io(bus, TW_SET_GCR, NULL);
}

/**
 * \brief Initializes the TWI structes.
 * \param sla Slave address to set.
 * \note The TWIBUS, TWIF and TWIHW will be initialized by this function.
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
	bus->twif->transfer = &BermudaTwiMasterTransfer;
	bus->twif->io = &BermudaTwIoctl;
	bus->twif->ifbusy = &BermudaTwHwIfacBusy;
	bus->twif->isr = &BermudaTwISR;

	// Initialize other parts the bus
	bus->busy = false;
#ifdef __EVENTS__
	bus->mutex = &twi0_mutex;
	bus->master_queue = &twi0_master_queue;
	bus->slave_queue = &twi0_slave_queue;
#elif __THREADS__
	bus->mutex = &tw_mutex;
	bus->master_queue = &tw_master_queue;
#endif

	// Initialize the hardware interface.
	bus->io.hwio = (void*)&twi0hw;
	((TWIHW*)bus->io.hwio)->io_in = AvrIO->pinc;
	((TWIHW*)bus->io.hwio)->io_out = AvrIO->portc;
	return bus;
}

/**
 * \brief Do TWI I/O control.
 * \param bus The TWI bus.
 * \param mode I/O control mode.
 * \param conf I/O configuration.
 * \return 0 on success, -1 when an invalid value in <b>mode</b> is given.
 * \see TW_IOCTL_MODE
 */
PRIVATE WEAK int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf)
{
	TWIHW *hw = bus->io.hwio;
	unsigned char sla;
	int rc = 0;
	BermudaEnterCritical();
	register unsigned char twcr = *(hw->twcr);
	BermudaExitCritical();
	
	switch(mode) {
		/* config cases */
		case TW_SET_RATE:
			*(hw->twbr) = *((unsigned char*)conf);
			break;
			
		case TW_SET_PRES:
			*(hw->twsr) = (*(hw->twsr) & (~B11)) | *((unsigned char*)conf);
			break;
			
		case TW_SET_SLA:
			sla = (*((unsigned char*)conf)) & ~(BIT(0)); // mask out the GCRE bit
			*(hw->twar) = sla;
			break;

		case TW_SET_GCR:
			sla = (*(hw->twar)) | BIT(0);
			*(hw->twar) = sla;
			break;

		/* bus control */
		case TW_GET_STATUS:
			bus->status = (*hw->twsr) & (~B111);
			*((unsigned char*)conf) = bus->status;
			break;
			
		case TW_BLOCK_INTERFACE:
			*(hw->twcr) = twcr & TW_BLOCK_MASK;
			break;
			
		case TW_RELEASE_BUS:
			*(hw->twcr) = TW_RELEASE;
			break;

		case TW_ENABLE_INTERFACE:
			*(hw->twcr) = BIT(TWINT) | TW_ENABLE;
			break;

		case TW_DISABLE_INTERFACE:
			*(hw->twcr) = twcr & TW_DISABLE_MASK;
			break;
			
		/* I/O cases */
		case TW_SENT_DATA:
		case TW_SENT_SLA:
			*(hw->twdr) = *( (unsigned char*)conf );
			*(hw->twcr) = TW_ACK;
			break;
			
		case TW_SENT_START:
			*(hw->twcr) = TW_START; // sent the given start
			break;

		case TW_SENT_STOP:
			*(hw->twcr) = TW_STOP; // enable the TWSTO bit
			break;
			
		case TW_READ_DATA:
			*((unsigned char*)conf) = *(hw->twdr);
			break;

		case TW_REPLY_ACK:
			*(hw->twcr) = TW_ACK;
			break;
		case TW_REPLY_NACK:
			*(hw->twcr) = TW_NACK;
			break;
			
		case TW_SLAVE_LISTEN:
			*(hw->twcr) = TW_LISTEN;
			
		default:
			rc = -1;
			break;
	}

	return rc;
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
 * \todo Implement the actual calculation.
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
	
	twbr = F_CPU / freq;
	twbr /= prescaler;
	twbr -= 16;
	twbr /= 2;
	
	return twbr;
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
unsigned char BermudaTwiCalcPres(uint32_t frq)
{
	unsigned char ret = 0;
	
	if(frq > TWI_FRQ(255,64) && frq < TWI_FRQ(255,16)) {
		ret = B11;
	}
	else if(frq > TWI_FRQ(255,16) && frq < TWI_FRQ(255,4)) {
		ret = B10;
	}
	else if(frq > TWI_FRQ(255,4) && frq < TWI_FRQ(255,1)) {
		ret = B1;
	}
	else if(frq > TWI_FRQ(255,1)) {
		ret = B0;
	}
	
	return ret;
}

/**
 * \brief Checks the status of SCL and SDA.
 * \param bus Bus interface to check.
 * \return -1 if the given interface is in idle; \n
 *         0 if SDA is low and SCL is HIGH; \n
 *         1 if SCL is low and SDA is high; \n
 *         2 if SCL and SDA are both low.
 * \return The default return value is 2.
 * 
 * It is safe to use the interface if this function returns -1 (both lines are
 * HIGH).
 */
PRIVATE WEAK int BermudaTwHwIfacBusy(TWIBUS *bus)
{
	TWIHW *hw = BermudaTwiIoData(bus);
	unsigned char scl = (*(hw->io_in)) & BIT(hw->scl);
	unsigned char sda = (*(hw->io_in)) & BIT(hw->sda);
	int rc = 2;
	
	switch(scl | sda) {
		case TW_IF_IDLE:
			rc = -1;
			break;
		case TW_IF_BUSY1:
			rc = 0;
			break;
		case TW_IF_BUSY2:
			rc = 1;
			break;
		case TW_IF_BUSY3:
			rc = 2;
			break;
	}
	
	return rc;
}

SIGNAL(TWI_STC_vect)
{
	twibus0->twif->isr(twibus0);
}
#endif /* __TWI__ */
