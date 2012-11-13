/*
 *  BermudaNet - Core layer
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
 * \file src/core/dev.c Core layer device file.
 * \addtogroup netCore Core layer
 * \brief The core layer is a protocol independant layer on top of the network driver to manage
 *        and route packets.
 * 
 * \section man Management
 * Packets are managed using netbuffs. Every packet has its own netbuff and has a theoretical unlimited
 * length. However, if the core layer finds out that the package is to big to transmit, it will
 * be fragmentated using the correct protocol fragmentation service. When receiving a fragmented packets, 
 * the core layer is also responsible for putting the packet back together.
 * 
 * The packets are fully managed by this layer, that also includes the memory management. Packets should
 * never be allocated and/or freed using the memory managers function calls. Always use netbuff_alloc
 * and netbuff_free.
 * 
 * @{
 */

#ifdef HAVE_CONFIG_H
#include <netconfig.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tokenbucket.h>

#include <dev/dev.h>
#include <dev/error.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <net/netbuff.h>
#include <net/netdev.h>

#include <net/core/dev.h>
#include <net/core/vlan.h>

/**
 * \brief Receive thread.
 * 
 * This thread will handle all incoming traffic.
 */
static THREAD tx_thread;

/**
 * \brief Transmit thread.
 * 
 * This thread will handle all outgoing traffic.
 */
static THREAD rx_thread;

/**
 * \brief Memory for the receive thread stack.
 */
static uint8_t tx_stack[NETIF_STACK_SIZ];

/**
 * \brief Memory for the transmit thread stack.
 */
static uint8_t rx_stack[NETIF_STACK_SIZ];

/**
 * \brief Root the list of network devices.
 */
static struct netdev *devRoot = NULL;

/* static functions */
static int __netif_init_dev(struct netdev *dev);

#ifdef __DOXYGEN__
/**
 * \brief Definition of the core layer processor thread.
 * \param netif_processor Name of the thread.
 * \param queue Name of the argument.
 */
THREAD_DEF(func netif_processor, struct netbuff_queue *queue);
#else
THREAD_DEF(netif_processor, queue);
#endif

/**
 * \brief Initialize the network core layer.
 * \param dev Atleast one network driver has to be active. Additional devices can be initialized
 *            using <i>netif_init_dev</i>.
 * \return <b>0</b> on success, if initialization failed <b>-1</b> will be returned.
 */
PUBLIC int netif_init(struct netdev *dev)
{
	if(devRoot) {
		return -DEV_ALREADY_INITIALIZED;
	}
	
	BermudaThreadCreate(&tx_thread, "netif_TX", &netif_processor, NULL, NETIF_STACK_SIZ, &tx_stack[0],
						BERMUDA_DEFAULT_PRIO);
	BermudaThreadCreate(&rx_thread, "netif_RX", &netif_processor, NULL, NETIF_STACK_SIZ, &rx_stack[0],
						BERMUDA_DEFAULT_PRIO);
	devRoot = dev;
	dev->next = NULL;
	return 0;
}

/**
 * \brief Initialize a new net device structure.
 * \param dev Partialy initialized net device.
 * \note The netdev.dev member has to point to a valid device structure.
 * \see struct device
 */
PUBLIC int netif_init_dev(struct netdev *dev)
{
	int rc = -1;
	struct netdev *car;
	
	for(car = devRoot; car != car->next && car != NULL; car = car->next) {
		if(car == dev) {
			rc = -DEV_ALREADY_INITIALIZED;
			break;
		} else {
			if(!strcmp(dev->name, car->name)) {
				rc = -DEV_ALREADY_INITIALIZED;
				break;
			}
			if(car->next == NULL) {
				car->next = dev;
				dev->next = NULL;
				rc = __netif_init_dev(dev);
				break;
			}
		}
	}
	
	return rc;
}

/**
 * \brief Initialize the hardware behind the netdev structure.
 * \param dev Device to initialize.
 * \return Error code from the device driver.
 * \see For errors see <i>dev_error</i>.
 */
static int __netif_init_dev(struct netdev *dev)
{
	return -1;
}

#ifdef __DOXYGEN__
/**
 * \brief Implementation of the core layer processor thread.
 * \param netif_processor Name of the thread.
 * \param queue Name of the argument.
 */
THREAD(int netif_processor, struct netbuff *queue);
#else
THREAD(netif_processor, queue)
{
	while(TRUE) {
		BermudaThreadSleep(100);
	}
}
#endif

/**
 * @}
 */
