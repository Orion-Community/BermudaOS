/*
 *  BermudaOS - USART device driver
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
#include <string.h>

#include <dev/dev.h>
#include <dev/usart/usart.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <sys/thread.h>
#include <sys/events/event.h>

PUBLIC int usartdev_socket(struct usartbus *bus, char *name, uint16_t flags)
{
	return bus->usartif->open(name);
}

PUBLIC int usartdev_close(int fd)
{
	struct usartbus *bus;
	FILE *stream = fdopen(fd);
	
	if(stream) {
		bus = stream->data;
		if(bus->usartif->close) {
			return bus->usartif->close(fd);
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}