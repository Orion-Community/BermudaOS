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
 * \todo Implement TWI.
 */

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/twif.h>

#include <arch/avr/io.h>
#include <arch/avr/328/dev/twibus.h>

#include <arch/avr/interrupts.h>

#ifdef __EVENTS__
/**
 * \var twi0_mutex
 * \brief Bus mutex.
 * 
 * Priority queue for bus 0.
 */
static volatile void *twi0_mutex = SIGNALED;

/**
 * \var twi0_queue
 * \brief Transfer queue.
 * 
 * Threads are placed in this queue when they are waiting for a TW transfer to
 * complete.
 */
static volatile void *twi0_queue;
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
	bus = BermudaHeapAlloc(sizeof(*bus));
	TWI0 = bus;
	bus->twif = BermudaHeapAlloc(sizeof( *(bus->twif) ));
	bus->twif->transfer = &BermudaTwiMasterTransfer;
	bus->twif->io = &BermudaTwIoctl;
	bus->twif->isr = &BermudaTwISR;
	bus->hwio = (void*)&twi0hw;
#ifdef __EVENTS__
	bus->mutex = &twi0_mutex;
	bus->queue = &twi0_queue;
#endif
	bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
	bus->twif->io(bus, TW_SET_SLA, &sla);
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
	TWIHW *hw = bus->hwio;
	unsigned char sla;
	int rc = 0;
	
	switch(mode) {
		/* config cases */
		case TW_SET_RATE:
			*(hw->twbr) = *((unsigned char*)conf);
			break;
			
		case TW_SET_PRES:
			*(hw->twsr) = (*(hw->twsr) &(~B11)) | *((unsigned char*)conf);
			break;
			
		case TW_SET_SLA:
			sla = (*((unsigned char*)conf)) & ~(BIT(0)); // mask out the GCRE bit
			*(hw->twar) = sla;
			break;
		
		/* bus control */
		case TW_GET_STATUS:
			bus->status = (*hw->twsr) & (~B111);
			*((unsigned char*)conf) = bus->status;
			break;
			
		case TW_RELEASE_BUS:
			*(hw->twcr) = TW_RELEASE;
			break;

		case TW_ENABLE_INTERFACE:
			*(hw->twcr) = TW_ENABLE;
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
 * \brief Transfer data using the twi bus.
 * \param twi Used TWI bus.
 * \param tx Transmit buffer.
 * \param rx Receive buffer.
 * \param tmo Transfer waiting time-out.
 * \warning A TWI init routine should be called before using this function.
 * \see BermudaTwi0Init
 * 
 * Data is transfered or received using the TWI bus. The mode this function
 * uses depends of the TWIMODE setting in the TWIBUS structure.
 */
PUBLIC int BermudaTwiMasterTransfer(bus, tx, txlen, rx, rxlen, sla, frq, tmo)
TWIBUS        *bus;
const void*   tx;
unsigned int  txlen;
void*         rx;
unsigned int  rxlen;
unsigned char sla;
uint32_t      frq;
unsigned int  tmo;
{
	int rc = -1;
#ifdef __EVENTS__
	if((rc = BermudaEventWait((volatile THREAD**)bus->mutex, tmo)) == -1) {
		goto out;
	}
	else if(tx == NULL && rx == NULL) {
		goto out;
	}
#else
#ifdef __THREADS__
	BermudaMutexEnter(&(bus->mutex));
#endif
	if(tx == NULL && rx == NULL) {
		goto out;
	}
#endif
	else {
		rc = 0;
	}
	
	BermudaTwInit(bus, tx, txlen, rx, rxlen, sla, frq);
	if(tx) {
		bus->mode = TWI_MASTER_TRANSMITTER;
	}
	else {
		bus->mode = TWI_MASTER_RECEIVER;
	}
	
	BermudaTwIoctl(bus, TW_SENT_START, NULL);
#ifdef __EVENTS__
	rc = BermudaEventWaitNext( (volatile THREAD**)bus->queue, tmo);
#endif

	out:
#ifdef __EVENTS__
	if(rc != -1) {
		BermudaEventSignal((volatile THREAD**)bus->mutex);
	}
#elif __THREADS__
	BermudaMutexRelease(&(bus->mutex));
#endif
	
	return rc;
}

/**
 * \brief Initialize the TWI bus.
 * \param twi Bus to initialize.
 * \param tx  Transmit buffer.
 * \param txlen Length of the transmit buffer.
 * \param rx Receive buffer.
 * \param rxlen Length of the received buffer.
 * \param sla Slave address used in the coming transmission.
 * \param frq Frequency to use in the coming transmission.
 * 
 * Used to initialize the TWI bus before starting a transfer.
 */
PRIVATE WEAK void BermudaTwInit(bus, tx, txlen, rx, rxlen, sla, frq)
TWIBUS*       bus;
const void*   tx;
unsigned int  txlen;
void*         rx;
unsigned int  rxlen;
unsigned char sla;
uint32_t      frq;
{
	bus->tx = tx;
	bus->txlen = txlen;
	bus->rx = rx;
	bus->rxlen = rxlen;
	bus->sla = sla;
	bus->freq = frq;
	
	if(frq) {
		unsigned char pres = BermudaTwiCalcPres(frq);
		unsigned char twbr = BermudaTwiCalcTWBR(frq, pres);
		BermudaTwIoctl(bus, TW_SET_RATE, &twbr);
		BermudaTwIoctl(bus, TW_SET_PRES, &pres);
	}
}

SIGNAL(TWI_STC_vect)
{
	twibus0->twif->isr(twibus0);
}
#endif /* __TWI__ */
