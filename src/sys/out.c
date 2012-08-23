/*
 *  BermudaOS - Printing functions
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

//! \file src/sys/out.c Text output module.

#include <stdio.h>
#include <bermuda.h>

#include <arch/usart.h>

#include <sys/events/event.h>

/**
 * \brief Bermudas implementation of printf.
 * \param fmt Format of the string to print.
 * \param ... Variable argument list.
 */
PUBLIC int BermudaPrintf(const char *fmt, ...)
{
#ifdef __EVENTS__
	if(BermudaEventWait((volatile THREAD**)USART0->mutex, 500) == -1) {
		return -1;
	}
#endif

	va_list va;
	int i;
	
	va_start(va, fmt);
	i = vfprintf(stdout, fmt, va);
	va_end(va);
#ifdef __EVENTS__
	BermudaEventSignal((volatile THREAD**)USART0->mutex);
#endif

	return i;
}