/*
 *  BermudaOS - LibC memcmp
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
#include <lib/string.h>

PUBLIC int memcmp(const void *s1, const void *s2, size_t n)
{
	if (n) {
	const unsigned char *p1 = s1, *p2 = s2;

		do {
			if (*p1++ != *p2++) {
				return (*--p1 - *--p2);
			}
		} while (--n);
	}
	return 0;
}