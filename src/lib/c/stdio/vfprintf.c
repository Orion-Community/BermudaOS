/*
File: printf.c

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <bermuda.h>
#include <stdio.h>
#include <stddef.h>

#include "stdio_priv.h"

#ifdef __AVR__
#include <arch/avr/pgm.h>
#endif

PUBLIC int vfprintf(FILE *stream, const char *fmt, va_list ap)
{
	size_t i;
	uint32_t uval;
	
	for(i = 0; fmt[i] != '\0'; i++) {
		if(fmt[i] != '%') {
			putc(fmt[i], stream);
		} else {
			fmt++;
			switch(fmt[i]) {
#if defined(PRINTF_FLT)
#warning "vfprintf does NOT support floating points yet!"
#endif
					
				case 'i':
					convert_to_num(va_arg(ap, unsigned int), 10, true, false, stream);
					break;
					
				case 'u':
					convert_to_num(va_arg(ap, unsigned int), 10, false, false, stream);
					break;
					
				case 'x':
					convert_to_num(va_arg(ap, unsigned int), 16, false, false, stream);
					break;
					
				case 'p':
				case 'X':
					if(fmt[i] == 'p') {
						uval = (size_t)va_arg(ap, void*);
					} else {
						uval = (uint32_t)va_arg(ap, unsigned int);
					}
				
					convert_to_num(uval, 16, false, true, stream);
					break;
					
				case 'c':
					putc((char)va_arg(ap, unsigned int), stream);
					break;
					
				case 's':
					fputs((char*)va_arg(ap, size_t), stream);
					break;
					
				case '%':
					putc('%', stream);
					break;
					
				default:
					break; /* oeps */
	}
		}
	}
	
	return stream->length;
}
