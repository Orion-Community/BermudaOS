/*
 *  BermudaOS - StdIO - I/O write progmem
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

#ifdef __AVR__
#include <stdlib.h>
#include <stdio.h>
#include <arch/avr/pgm.h>

/**
 * \brief Write program memory to a file.
 * \param fd File descriptor.
 * \param data Program memory pointer.
 * \param size Length of data.
 * \warning This function cannot be used with assynchronous drivers (such as I2C and SPI). Check
 *          the device driver documentation to see wether it's safe to use this function or not.
 */
PUBLIC int write_P(int fd, const void *data, size_t size)
{
	void *data2;
	int rc = -1;
	
	if((data2 = BermudaHeapAlloc(size)) != NULL) {
		memcpy_P(data2, data, size);
		rc = write(fd, data2, size);
		BermudaHeapFree(data2);
	}
	
	return rc;
	
	
}
#endif
