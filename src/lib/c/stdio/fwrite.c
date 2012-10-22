/*
 *  BermudaOS - StdIO - I/O write
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

PUBLIC int fwrite(FILE *stream, const void *buff, size_t size)
{
	if(stream) {
		if((stream->flags & __SWR) != 0 && stream->write) {
			return stream->write(stream, buff, size);
		} else {
			return -2;
		}
	} else {
		return -1;
	}
}