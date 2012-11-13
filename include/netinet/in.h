/*
 *  BermudaNet - Network interface utillity
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
 * \file in.h
 * \brief Network utillity functions.
 * 
 * <i><b>in.h</b></i> provides utillity functions to interface with the network.
 */

#ifndef __NETIF_IN_H
#define __NETIF_IN_H

#include <stdlib.h>

/**
 * \brief Host to network order (word conversion).
 * \param word Word to convert from host to network order.
 * \return Network endian value of <i>word</i>.
 */
static __16be htons(uint16_t word)
{
	return __byte_swap2(word);
}

/**
 * \brief Host to network order (double word conversion).
 * \param dword Double word to convert from host to network order.
 * \return Network endian value of <i>word</i>.
 */
static __32be htonl(uint32_t dword)
{
	return __byte_swap4(dword);
}

/**
 * \brief Network to host order (word conversion).
 * \param word Word to convert from network to host order.
 * \return Host endian value of <i>word</i>.
 */
static uint16_t ntohs(__16be word)
{
	return __byte_swap2(word);
}

/**
 * \brief Network to host order (double word conversion).
 * \param dword Double word to convert from network to host order.
 * \return Host endian value of <i>dword</i>.
 */
static uint32_t ntohl(__32be dword)
{
	return __byte_swap4(dword);
}

#endif /* __NETIF_IN_H */