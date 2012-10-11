/*
 *  BermudaOS - putc
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

#include "stdio_priv.h"

PUBLIC int vfprintf_P(FILE *stream, const char *fmt, va_list ap)
{
	size_t strl = strlen_P(fmt)+1;
	int rc = -1;
	
	char *fmt2 = BermudaHeapAlloc(strl);
	
	if(fmt2) {
		memcpy_P(fmt2, fmt, strl);
		rc = vfprintf(stream, fmt2, ap);
		BermudaHeapFree(fmt2);
	}
	
	return rc;
}
#endif
