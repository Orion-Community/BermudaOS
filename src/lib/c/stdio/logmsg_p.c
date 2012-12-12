/*
 *  BermudaOS - StdIO - logmsg_P
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
#include <stdio.h>

#include <arch/avr/pgm.h>

PUBLIC int logmsg_P(FILE *stream, const char *origin, const char *fmt, ...)
{
	int rc;
	va_list va;
	
	fprintf_P(stream, PSTR("%s: "), origin);
	
	va_start(va, fmt);
	rc = vfprintf_P(stream, fmt, va);
	va_end(va);
	
	return rc;
}