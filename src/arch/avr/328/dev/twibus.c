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

/**
 * \file arch/avr/328/dev/twibus.c Hardware TWI bus controller.
 * \todo Implement TWI.
 */

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/twif.h>
#include <arch/avr/328/dev/twibus.h>

#include <avr/interrupt.h>

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
static volatile void *twi0_queue = SIGNALED;
#endif

/**
 * \var twibus0
 * \brief Global pointer to the TWI0 bus.
 * \see TWI0
 */
static TWIBUS *twibus0 = NULL;

/**
 * \brief Initialize TWI bus 0.
 * \param bus Bus structure pointer. Should not be NULL.
 */
PUBLIC void BermudaTwi0Init(TWIBUS *bus)
{
	if(bus == NULL) {
		return;
	}
	
	twibus0 = bus;
	bus->twif = BermudaHeapAlloc(sizeof( *(bus->twif) ));
	bus->twif->transfer = &BermudaTwiMasterTransfer;
	bus->twif->io = &BermudaTwIoctl;
	bus->twif->isr = &BermudaTwISR;
#ifdef __EVENTS__
	bus->mutex = &twi0_mutex;
	bus->queue = &twi0_queue;
#endif
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
	register unsigned char twcr;
	int rc = 0;
	
	switch(mode) {
		case TW_SET_RATE:
			*(hw->twbr) = *((unsigned char*)conf);
			break;
		case TW_SET_PRES:
			*(hw->twsr) = (*(hw->twsr) &(~B11)) | *((unsigned char*)conf);
			break;
		case TW_SET_SLA:
			sla = (*((unsigned char*)conf)) << 1; // shift out the GCRE bit
			*(hw->twar) = sla;
			break;

		case TW_SENT_DATA:
		case TW_SENT_SLA:
			*(hw->twdr) = *( (unsigned char*)conf );
			BermudaEnterCritical();
			twcr = *(hw->twcr) & (~BIT(5)); // disable TW start.
			*(hw->twcr) = TW_ENABLE | twcr;
			BermudaExitCritical();
			break;
		case TW_START:
			*(hw->twcr) = (*(unsigned char*)conf); // sent the given start
			break;
		case TW_GET_STATUS:
			bus->status = (*hw->twsr) & (~B111);
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
 *       * TW_PRES_64 \n
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
 *         * B0 for a prescaler of 1;
 *         * B1 for a prescaler of 4;
 *         * B10 for a prescaler of 16;
 *         * B11 for a prescaler of 64
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
PUBLIC int BermudaTwiMasterTransfer(twi, tx, txlen, rx, rxlen, sla, frq, tmo)
TWIBUS        *twi;
const void*   tx;
unsigned int  txlen;
void*         rx;
unsigned int  rxlen;
unsigned char sla;
uint32_t      frq;
unsigned int  tmo;
{
	int rc = -1;
	unsigned char start = TWGO;
#ifdef __EVENTS__
	if((rc = BermudaEventWait((volatile THREAD**)twi->mutex, tmo)) == -1) {
		goto out;
	}
	else if(tx == NULL && rx == NULL) {
		goto out;
	}
#else
	if(tx == NULL && rx == NULL) {
		goto out;
	}
#endif
	else {
		rc = 0;
	}
	
	BermudaTwInit(twi, tx, txlen, rx, rxlen, sla, frq);
	if(rx == NULL || tx != NULL) {
		twi->mode = TWI_MASTER_TRANSMITTER;
	}
	else if(tx == NULL || rx != NULL) {
		twi->mode = TWI_MASTER_RECEIVER;
	}
	
	BermudaTwIoctl(twi, TW_START, &start);
	rc = BermudaEventWait( (volatile THREAD**)twi->queue, tmo);

	out:
#ifdef __EVENTS__
	if(rc != -1) {
		BermudaEventSignal((volatile THREAD**)twi->mutex);
	}
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
PRIVATE WEAK void BermudaTwInit(twi, tx, txlen, rx, rxlen, sla, frq)
TWIBUS*       twi;
const void*   tx;
unsigned int  txlen;
void*         rx;
unsigned int  rxlen;
unsigned char sla;
uint32_t      frq;
{
	twi->tx = tx;
	twi->txlen = txlen;
	twi->rx = rx;
	twi->rxlen = rxlen;
	twi->sla = sla;
	twi->freq = frq;
	
	if(frq) {
		unsigned char pres = BermudaTwiCalcPres(frq);
		unsigned char twbr = BermudaTwiCalcTWBR(frq, pres);
		BermudaTwIoctl(twi, TW_SET_RATE, &twbr);
		BermudaTwIoctl(twi, TW_SET_PRES, &pres);
	}
}

SIGNAL(TWI_vect)
{
	twibus0->status = BermudaTwIoctl(twibus0, TW_GET_STATUS, NULL);
	twibus0->twif->isr(twibus0);
}
