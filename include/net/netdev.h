/*
 *  BermudaNet - Net device header
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

#include <dev/dev.h>

#ifndef __NETDEV_H
#define __NETDEV_H

/**
 * \def NETDEV_NAME_SIZ
 * \brief Defines the size of the name unique name identifier in the netdev structure.
 */
#define NETDEV_NAME_SIZ 6

struct netdev
{
	struct netdev *next;

	char name[NETDEV_NAME_SIZ]; //!< Unique name identifier.
	uint16_t mtu; //!< Maximum transmissable unit size which is supported by this device.
	size_t iobase;
	
	volatile void *tx_queue;
	volatile void *rx_queue;
};

#endif
