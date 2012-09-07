/*
 *  BermudaOS - USART interface
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

#ifndef __USART_INTERFACE_H_
#define __USART_INTERFACE_H_

#include <bermuda.h>

#include <arch/types.h>

/**
 * \def USART_RX
 * \brief Definition of the USART receive transfer type.
 * \see BermudaUsartISR
 */
#define USART_RX 0

/**
 * \def USART_TX
 * \brief Definition of the USART transmit transfer type.
 * \see BermudaUsartISR
 */
#define USART_TX 1

/**
 * \typedef USARTBUS
 * \brief Type definition of USARTBUS.
 *
 * Defines the type for USARTBUS to struct _usartbus.
 */
typedef struct _usartbus USARTBUS;

/**
 * \typedef USARTIF
 * \brief Type definition of USARTIF.
 *
 * Defines the type for USARTIF to struct _usartif.
 */
typedef struct _usartif  USARTIF;

typedef enum
{
	USART_SET_BAUD, //! Set the baud rate.
	
	USART_START, //!< Start the USART controller.
	USART_STOP, //!< Stops the USART controller.
	
	USART_TX_DATA, //!< Transmit data. 
	USART_RX_DATA, //!< Read data from the backend buffer.
} USART_IOCTL_MODE;

/**
 * \brief The _usartif structure defines the USART interface.
 *
 * This structure defines the interface (or communication) to other parts of the
 * U(S)ART module.
 */
struct _usartif
{
	int (*transfer)(USARTBUS *bus, const void *tx, uptr tx_len, void *rx, uptr rx_len,
					unsigned int baud, int tmo);
	void (*io)(USARTBUS *bus, USART_IOCTL_MODE mode, void *data);
	void (*isr)(USARTBUS *bus, unsigned char transtype);
	int (*ifbusy)(USARTBUS *bus);
} __attribute__((packed));

/**
 * \brief Structure definition of the USART.
 * \see _usartif for the communication with the hardware.
 *
 * The USART is defined by this structure.
 */
struct _usartbus
{
#ifdef __EVENTS__
	void *mutex; //!< Bus mutex.
	void *tx_queue; //!< Transmit waiting queue.
	void *rx_queue; //!< Receive waiting queue.
#elif __THREADS__
	mutex_t mutex;  //!< Bus mutex.
	mutex_t queue; //!< Transfer mutex.
#endif
	union {
		void *hwio;   //!< Hardware I/O pointer.

		/**
		 * \brief Pointer to the software USART I/O structere.
		 * \note Has not been defined yet.
		 * \todo Define the USART software I/O structe.
		 */
		void *softio;
	} io;

	USARTIF *usartif;

	const unsigned char *tx; //!< Transmit buffer.
	uptr tx_len;                //!< Transmit buffer length.
	uptr tx_index;              //!< Transmit buffer index.
	
	unsigned char *rx; //!< Receive buffer.
	uptr rx_len;                //!< Receive buffer length.
	uptr rx_index;              //!< Receive buffer index.
} __attribute__((packed));

extern void BermudaUsartISR(USARTBUS *bus, unsigned char transtype);
extern int BermudaUsartTransfer(USARTBUS *bus, const void *tx, unsigned int txlen, 
								unsigned int baud, unsigned int tmo);
extern int BermudaUsartListen(USARTBUS *bus, void *rx, unsigned int rxlen,
							  unsigned int baud, unsigned int tmo);


#endif /* __USART_INTERFACE_H_ */