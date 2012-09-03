/*
 *  BermudaOS - AVR ATmega TWI module
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
 * \file src/arch/avr/twif.c ATmega Two Wire Interface.
 * \brief Implementation of the TWI/I2C for the AVR ATmega processor family.
 */

#if defined(__TWI__) || defined(__DOXYGEN__)

#include <bermuda.h>

#include <lib/binary.h>
#include <dev/twif.h>
#include <arch/twi.h>

#include <sys/events/event.h>

// private function declarations
PRIVATE WEAK void BermudaTwInit(TWIBUS *twi, const void *tx, unsigned int txlen, 
	void *rx, unsigned int rxlen, unsigned char sla, uint32_t frq);

/**
 * \brief Generic TWI interrupt handler.
 * \param bus TWI bus which raised the interrupt.
 * \warning Should only be called by hardware!
 * \note When addressed as a slave receiver, the bus is blocked after the STOP
 *       bit. The user should always respond with BermudaTwiSlaveRespond.
 * 
 * Generic handling of the TWI logic. It will first sent all data in the transmit
 * buffer, if present. Then it will receive data in the receive buffer, if a
 * Rx buffer address is configured.
 */
PUBLIC __link void BermudaTwiISR(TWIBUS *bus)
{
	unsigned char sla = bus->sla & (~BIT(0));
	TW_IOCTL_MODE mode;
	unsigned char dummy = 0;
	bus->twif->io(bus, TW_GET_STATUS, (void*)&(bus->status));
	
	switch(bus->status) {
		/* master start */
		case TWI_MASTER_REP_START:
		case TWI_MASTER_START:
			// TWI start signal has been sent. The interface is ready to sent
			// the slave address we want to address.
			bus->master_index = 0;
			bus->busy = true;
			if(bus->mode == TWI_MASTER_RECEIVER) {
				sla |= 1; // SLA+R
			}

			bus->twif->io(bus, TW_SENT_SLA, &sla);
			break;
		
		/*
		 * Transmit data as a master transmitter to the the addressed slave
		 * device.
		 */
		case TWI_MT_SLA_ACK: // slave ACKed SLA+W
		case TWI_MT_DATA_ACK: // slave ACKed data
			if(bus->master_index < bus->master_tx_len) {
				bus->twif->io(bus, TW_SENT_DATA, (void*)&(bus->master_tx[bus->master_index]));
				bus->master_index++; // late increment for AVRICC
			}
			else if(bus->master_rx_len) {
				bus->mode = TWI_MASTER_RECEIVER;
				bus->twif->io(bus, TW_SENT_START, NULL); // sent repeated start
				bus->master_tx_len = 0;
			}
			else { // end of transfer
				bus->error = E_SUCCESS;
				bus->twif->io(bus, TW_SENT_STOP, NULL);
#ifdef __EVENTS__
				BermudaEventSignalFromISR( (volatile THREAD**)bus->master_queue);
#elif __THREADS__
				BermudaMutexRelease(&(bus->master_queue));
#endif
				bus->busy = false;
				bus->master_tx_len = 0;
				
				/*
				 * Enable listening for slave requests if there is a slave
				 * operation waiting.
				 */
				if(bus->slave_rx_len) {
					bus->twif->io(bus, TW_SLAVE_LISTEN, NULL);
				}
			}
			break;
		
		/*
		 * Data is transmitted as a master transmitter and a NACK is returned.
		 * If there is no arbitration a STOP bit will be generated on the bus.
		 */
		case TWI_MT_SLA_NACK: // slave NACKed SLA+W
		case TWI_MT_DATA_NACK: // slave NACKed data byte
			bus->master_tx_len = 0;
			/* roll through */
		case TWI_MR_SLA_NACK: // SLA+R sent but NACKed
		case TWI_MASTER_ARB_LOST: // lost bus control
			if(bus->status == TWI_MASTER_ARB_LOST) {
				mode = TW_RELEASE_BUS;
			}
			else {
				mode = TW_SENT_STOP;
			}
			bus->error = bus->status;
			bus->twif->io(bus, mode, NULL);
			bus->busy = false;
			bus->master_rx_len = 0;
			
			/*
			 * Start wait for slave requests if possible
			 */
			if(bus->slave_rx_len) {
				bus->twif->io(bus, TW_SLAVE_LISTEN, NULL);
			}
			else {
				bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
			}
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->master_queue);
#elif __THREADS__
			BermudaMutexRelease(&(bus->master_queue));
#endif
			break;
		
		/*
		 * Received data as a master and returned an ACK to the sending slave.
		 */
		case TWI_MR_DATA_ACK:
			if(bus->master_index < bus->master_rx_len) {
				bus->twif->io(bus, TW_READ_DATA, (void*)&(bus->master_rx[bus->master_index]));
				bus->master_index++;
			}
			// fall through to sent data
		case TWI_MR_SLA_ACK: // slave ACKed SLA+R
			if(bus->master_index + 1 < bus->master_rx_len) {
				/*
				 * Only enable ACKing if we are not receiving the last data byte.
				 */
				bus->twif->io(bus, TW_REPLY_ACK, NULL);
			}
			else {
				bus->twif->io(bus, TW_REPLY_NACK, NULL);
			}
			break;
			
		/*
		 * Received data as a master receiver and returned a NACK. The received
		 * byte will be stored and a STOP bit will be generated. The user application
		 * will also be waken up if threading/events are enabled.
		 */
		case TWI_MR_DATA_NACK:
			if(bus->master_index < bus->master_rx_len) {
				bus->twif->io(bus, TW_READ_DATA, (void*)&(bus->master_rx[bus->master_index]));
			}
			
			bus->error = bus->status;
			bus->twif->io(bus, TW_SENT_STOP, NULL);
			bus->busy = false;
			bus->master_rx_len = 0;
			
			if(bus->slave_rx_len) {
				bus->twif->io(bus, TW_SLAVE_LISTEN, NULL);
			}
			else {
				bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
			}
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->master_queue);
#elif __THREADS__
			BermudaMutexRelease(&(bus->master_queue));
#endif
			break;

		/*
		 * TWI_SR_SLAW_ACK: Own slave address received, ack returned.
		 * TWI_SR_GC_ACK: General call received, ACK returned.
		 * TWI_SR_GC_ARB_LOST: Lost arbitration as master in SLA+R/W,
		 *                     received generall call.
		 * TWI_SR_SLAW_ARB_LOST: Lost arbitration as master in SLA+R/W,
		 *                       received own slave address.
		 */
		case TWI_SR_SLAW_ACK:
		case TWI_SR_GC_ACK:
		case TWI_SR_GC_ARB_LOST:
		case TWI_SR_SLAW_ARB_LOST:
			if(bus->slave_rx_len) {
				mode = TW_REPLY_ACK;
				bus->slave_index = 0; // reset receive buffer.
				bus->busy = true;
			}
			else {
				mode = TW_REPLY_NACK;
				bus->error = bus->status;
			}
			bus->twif->io(bus, mode, NULL);
			break;

		/*
		 * Previously addressed with own SLA+W or GC; data has been received and
		 * ACKed.
		 */
		case TWI_SR_SLAW_DATA_ACK:
		case TWI_SR_GC_DATA_ACK:
			if(bus->slave_index < bus->slave_rx_len) {
				bus->twif->io(bus, TW_READ_DATA, (void*)&(bus->slave_rx[bus->slave_index]));
				if(bus->slave_index + 1 < bus->slave_rx_len) {
					// if there is space for at least one more byte
					bus->twif->io(bus, TW_REPLY_ACK, NULL);
				}
				else {
					// no more space in buffer, nack next incoming byte
					bus->twif->io(bus, TW_REPLY_NACK, NULL);
				}
				bus->slave_index++;
				break;
			}
		
		/*
		 * Previously addressed with own SLA+W or GC; data has been received and
		 * NACKed.
		 */
		case TWI_SR_SLAW_DATA_NACK:
		case TWI_SR_GC_DATA_NACK:
			if(bus->master_index) {
				/*
				 * If there is a master operation waiting, start it as soon as
				 * the bus becomes available.
				 */
				bus->twif->io(bus, TW_SENT_START, NULL);
			}
			else {
				/* 
				 * Continue to deny data from the master.
				 */
				bus->twif->io(bus, TW_REPLY_NACK, NULL); // NACK and wait for stop
			}
			break;

		/*
		 * The current bus master did sent a stop condition to terminate the
		 * transfer. The bus will be blocked blocked by holding SCL low. Bus
		 * operation can be continued by calling BermudaTwiSlaveRespond.
		 */
		case TWI_SR_STOP:
			bus->error = TWI_SR_STOP;
			bus->busy = false;
			bus->twif->io(bus, TW_BLOCK_INTERFACE, NULL);
			bus->slave_rx_len = 0;
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->slave_queue);
#elif __THREADS__
			BermudaMutexRelease(&(bus->slave_queue));
#endif
			break;
			
		/*
		 * TWI entered slave transmitter mode.
		 */
		case TWI_ST_ARB_LOST:
		case TWI_ST_SLAR_ACK:
			bus->slave_index = 0;
			bus->busy = true;
		
		/*
		 * Data is sent and ACKed.
		 */
		case TWI_ST_DATA_ACK:
			if(bus->slave_index < bus->slave_tx_len) {
				bus->twif->io(bus, TW_SENT_DATA, (void*)&(bus->slave_tx[bus->slave_index]));
				if(bus->slave_index+1 < bus->slave_tx_len) {
					mode = TW_REPLY_ACK;
				}
				else {
					mode = TW_REPLY_NACK;
				}
				bus->slave_index++;
				bus->twif->io(bus, mode, NULL);
				break;
			}
			else {
				bus->twif->io(bus, TW_SENT_DATA, &dummy);
				bus->twif->io(bus, TW_REPLY_NACK, NULL);
				break;
			}
			
		/*
		 * TWI_ST_DATA_NACK: Transmitted a data byte to the master and received a
		 *                   NACK back from the master.
		 * TWI_ST_LAST_DATA_ACK: The last data byte has been received.
		 * 
		 * Seize the transmission.
		 */
		case TWI_ST_DATA_NACK:
		case TWI_ST_LAST_DATA_ACK:
			bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
			bus->error = bus->status;
			bus->busy = false;
			bus->slave_tx_len = 0;
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->slave_queue);
#elif __THREADS__
			BermudaMutexRelease(&(bus->slave_queue));
#endif
			if(bus->master_tx_len || bus->master_rx_len) {
				bus->twif->io(bus, TW_SENT_START, NULL);
			}
			break;
			
		case TWI_BUS_ERROR:
		default:
			bus->error = E_GENERIC;
			bus->slave_index = 0;
			bus->master_index = 0;
			
			bus->master_rx_len = 0;
			bus->master_tx_len = 0;
			bus->slave_rx_len = 0;
			bus->slave_tx_len = 0;
			
			bus->twif->io(bus, TW_RELEASE_BUS, NULL);
			bus->busy = false;
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->master_queue);
			BermudaEventSignalFromISR( (volatile THREAD**)bus->slave_queue);
#elif __THREADS__
			BermudaMutexRelease(&(bus->master_queue));
			BermudaMutexRelease(&(bus->slave_queue));
#endif
			break;
	}
	
	return;
}

/**
 * \brief Transfer data using the twi bus.
 * \param twi Used TWI bus.
 * \param tx Transmit buffer.
 * \param txlen Transmit buffer length.
 * \param rx Receive buffer.
 * \param rxlen Receive buffer length.
 * \param sla Slave address to sent to.
 * \param frq Frequency to use when sending data.
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
	
	if(bus->busy == false && bus->twif->ifbusy(bus) == -1) {
		bus->twif->io(bus, TW_SENT_START, NULL);
	}
#ifdef __EVENTS__
	rc = BermudaEventWaitNext( (volatile THREAD**)bus->master_queue, tmo);
#elif __THREADS__
	BermudaMutexEnter(&(bus->master_queue));
#endif


out:
	bus->master_tx_len = 0;
	bus->master_rx_len = 0;
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
	bus->master_tx = tx;
	bus->master_tx_len = txlen;
	bus->master_rx = rx;
	bus->master_rx_len = rxlen;
	bus->sla = sla;
	bus->freq = frq;
	
	if(frq) {
		unsigned char pres = BermudaTwiCalcPres(frq);
		unsigned char twbr = BermudaTwiCalcTWBR(frq, pres);
		bus->twif->io(bus, TW_SET_RATE, &twbr);
		bus->twif->io(bus, TW_SET_PRES, &pres);
	}
}

/**
 * \brief Listen for requests.
 * \param bus TWI bus.
 * \param num Will be set to the amount of received bytes.
 * \param rx Rx buffer.
 * \param rxlen Rx buffer length.
 * \param tmo Listen time-out.
 * \return Error code. 0 for success.
 * \warning When this function returns without error, the bus is blocked!
 * \warning If this function responds without error, the application should
 *          respond IMMEDIATLY with BermudaTwiSlaveRespond.
 * 
 * Listens for requests by a master to this TWI bus interface.
 */
PUBLIC int BermudaTwiSlaveListen(TWIBUS *bus, uptr *num, void *rx, uptr rxlen, 
	unsigned int tmo)
{
	int rc = -1;
	
	BermudaEnterCritical();

	bus->slave_rx = rx;
	bus->slave_rx_len = rxlen;
	
	if(bus->busy == false) {
		if((bus->master_rx_len || bus->master_tx_len)  && 
			bus->twif->ifbusy(bus) == -1) {
			bus->twif->io(bus, TW_SENT_START, NULL);
		}
		else {
			bus->twif->io(bus, TW_SLAVE_LISTEN, NULL);
		}
	}
	BermudaExitCritical();
	
	if((rc = BermudaEventWaitNext( (volatile THREAD**)bus->slave_queue, tmo))) {
		bus->error = E_TIMEOUT;
	}
	
	if(bus->error == TWI_SR_STOP) {
		*num = bus->slave_index;
	}
	bus->slave_rx_len = 0;
	return rc;
}

/**
 * \brief Respond to a TWI slave request.
 * \param bus TWI bus.
 * \param tx Transmit buffer.
 * \param txlen Transmit buffer length.
 * \param tmo Time-out.
 * \note tmo Should not be set to BERMUDA_EVENT_WAIT_INFINITE.
 * \see BERMUDA_EVENT_WAIT_INFINITE
 * 
 * If tx and txlen are set to 0, no data will be transmitted and the bus
 * will be released.
 */
PUBLIC int BermudaTwiSlaveRespond(TWIBUS *bus, const void *tx, uptr txlen,
	unsigned int tmo)
{
	int rc = -1;

	if(tx && txlen) { // if there is something to transmit
		BermudaEnterCritical();

		bus->slave_index = 0;
		bus->slave_tx = tx;
		bus->slave_tx_len = txlen;

		bus->twif->io(bus, TW_SLAVE_LISTEN, NULL); // release the bus
		BermudaExitCritical();

		if((rc = BermudaEventWaitNext((volatile THREAD**)bus->slave_queue, tmo))) {
			bus->error = E_TIMEOUT;
		}
	}
	else if(bus->master_tx_len || bus->master_rx_len) {
		bus->twif->io(bus, TW_SENT_START, NULL);
	}
	else {
		bus->busy = false;
		bus->twif->io(bus, TW_ENABLE_INTERFACE, NULL);
	}

	return rc;
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
PUBLIC int BermudaTwHwIfacBusy(TWIBUS *bus)
{
	HWTWI *hw = BermudaTwiIoData(bus);
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

/**
 * \brief Do TWI I/O control.
 * \param bus The TWI bus.
 * \param mode I/O control mode.
 * \param conf I/O configuration.
 * \return 0 on success, -1 when an invalid value in <b>mode</b> is given.
 * \see TW_IOCTL_MODE
 */
PUBLIC int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf)
{
	HWTWI *hw = BermudaTwiIoData(bus);
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
			*(hw->twcr) = twcr & (~BIT(TWINT) | BIT(TWIE));
			break;
			
		case TW_ENABLE_INTERFACE:
		case TW_RELEASE_BUS:
			*(hw->twcr) = twcr | (BIT(TWEN) | BIT(TWINT) | BIT(TWIE));
			break;

		case TW_DISABLE_INTERFACE:
			*(hw->twcr) = twcr & (~BIT(TWINT) & ~BIT(TWEN) & ~BIT(TWEA) & 
						~BIT(TWIE) & ~BIT(TWSTA));
			break;
			
		/* I/O cases */
		case TW_SENT_DATA:
		case TW_SENT_SLA:
			*(hw->twdr) = *( (unsigned char*)conf );
			*(hw->twcr) = (twcr & ~BIT(TWSTA)) | (BIT(TWEN) | BIT(TWINT) | 
							BIT(TWIE) | BIT(TWEA));
			break;
			
		case TW_SENT_START:
			*(hw->twcr) = twcr | (BIT(TWEN) | BIT(TWINT) | BIT(TWIE) | 
							BIT(TWEA) | BIT(TWSTA)); // sent the given start
			break;

		case TW_SENT_STOP:
			*(hw->twcr) = twcr | (BIT(TWEN) | BIT(TWINT) | BIT(TWIE) |
							BIT(TWSTO)); // enable the TWSTO bit
			break;
			
		case TW_READ_DATA:
			*((unsigned char*)conf) = *(hw->twdr);
			break;

		case TW_REPLY_ACK:
			*(hw->twcr) = twcr | (BIT(TWEN) | BIT(TWINT) | BIT(TWIE) | BIT(TWEA));
			break;
			
		case TW_REPLY_NACK:
			*(hw->twcr) = BIT(TWEN) | BIT(TWINT) | BIT(TWIE);
			break;
			
		case TW_SLAVE_LISTEN:
			*(hw->twcr) = twcr | (BIT(TWEN) | BIT(TWINT) | BIT(TWIE) | BIT(TWEA));
			
		default:
			rc = -1;
			break;
	}

	return rc;
}

/**
 * \brief Destroys the TW bus structures.
 * \param bus Bus to destroy.
 * \param type Defines the type of the bus (hardware/software).
 */
PUBLIC void BermudaTwiBusFactoryDestroy(TWIBUS *bus, TWI_BUS_TYPE type)
{
	BermudaHeapFree(bus->twif);
	if(type == TWI_SOFTWARE_CONTROLLER) {
		BermudaHeapFree(bus->io.softio);
	}
	BermudaHeapFree(bus);
}
#endif /* __TWI__ */
