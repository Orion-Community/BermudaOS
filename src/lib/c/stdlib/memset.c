/*
 *  BermudaOS - Memset
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

#include <stdlib.h>

/**
 * \brief Fill a memory region with a byte value.
 * \param dst Memory region pointer.
 * \param c Byte value.
 * \param n Number of bytes.
 * \retval \p dst: the original memory region pointer.
 */
PUBLIC void *memset(void *dst, int c, size_t n)
{
	if (n) {
		char *d = dst;

		do {
			*d++ = c;
		} while (--n);
	}
	return dst;
}