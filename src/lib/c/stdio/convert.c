/*
 *  BermudaOS - StdIO - I/O number conversion
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

#include "stdio_priv.h"

PUBLIC int convert_to_num(uint32_t num, uint8_t base, bool sign, 
							  bool caps, FILE *stream)
{
	char buff[BUFF];
	char *cp;
	char *digx;
	//memset(cp, 0xa, BUFF-1UL);

	if(num == 0) {
		putc('0', stream);
		return 0;
	} else if((int32_t)num < 0 && sign) {
		putc('-', stream);
	}
	
	if(caps) {
		digx = "0123456789ABCDEF";
	} else {
		digx = "0123456789abcdef";
	}

	cp = buff + BUFF - 1UL;
	*cp = '\0';
	if(base == 16) {
		do {
			*--cp = digx[num & 0x0F];
			num >>= 4;
		} while(num);
	} else if(base == 10) {
		if(num < 10) {
			*--cp = (char)num + '0';
		} else {
			do {
				*--cp = (char)(num % 10) + '0';
				num /= 10;
			} while(num);
		}
	}
	
	fputs(cp, stream);
	return 0;
}
