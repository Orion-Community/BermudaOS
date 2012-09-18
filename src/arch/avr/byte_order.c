/*
 *  BermudaOS - Byte order functions
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

/**
 * \brief AVR implementation of the host to network order function.
 * \param val 16-bit value to convert to network order (big endian).
 */
__16be htons(uint16_t val)
{
	uint16_t tmp = (val & 0xFF) << 8;
	val >>= 8;
	return (tmp | val);
}

/**
 * \brief AVR implementation of the network to host order (little endian) function.
 * \param val 16-bit value to convert to host order (little endian).
 */
uint16_t ntohs(__16be val)
{
	return htons(val);
}

/**
 * \brief AVR implementation of the host to network order function.
 * \param val 32-bit value to convert to network order (big endian).
 */
__32be htonl(uint32_t val)
{
	uint32_t tmp = 0;
	char i = sizeof(uint32_t)-1;
	
	for(; i >= 0; i--) {
		tmp |= (val & 0xFF) << (i*8);
		val >>= 8;
	}
	return tmp;
}

/**
 * \brief AVR implementation of the network to host order (little endian) function.
 * \param val 32-bit value to convert to host order (little endian).
 */
uint32_t ntohl(__32be val)
{
	return htonl(val);
}
