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

#include <arch/usart.h>

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

/**
 * \brief The _usartif structure defines the USART interface.
 *
 * This structure defines the interface (or communication) to other parts of the
 * U(S)ART module.
 */
struct _usartif
{
	int transfer(USARTBUS *bus, const void *tx, uprt tx_len, void *rx, uptr rx_len,
				 unsigned int baud, int tmo);
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
	void *queue; //!< Transfer waiting queue.
#elif __THREADS__
	mutex_t mutex;  //!< Bus mutex.
	mutext_t queue; //!< Transfer mutex.
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

	volatile unsigned char *tx; //!< Transmit buffer.
	uptr tx_len;                //!< Transmit buffer length.
	uptr tx_index;              //!< Transmit buffer index.
	
	volatile unsigned char *rx; //!< Receive buffer.
	uptr rx_len;                //!< Receive buffer length.
	uptr rx_index;              //!< Receive buffer index.
} __attribute__((packed));

#endif /* __USART_INTERFACE_H_ */