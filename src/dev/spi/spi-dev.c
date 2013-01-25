/*
 *  BermudaOS - SPI device interface
 *  Copyright (C) 2012   Michel Megens <dev@michelmegens.net>
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

#include <dev/dev.h>
#include <dev/spi.h>
#include <dev/error.h>

/**
 * \brief Create a SPI socket.
 * \param client SPI chip client.
 * \param flags I/O flags.
 */
PUBLIC int spidev_socket(struct spi_client *client, uint16_t flags)
{
	int rc;
	FILE *stream = malloc(sizeof(*stream));
	
	if(!stream) {
		return -1;
	}
	
	rc = iob_add(stream);
	if(rc < 0) {
		free(stream);
		return -1;
	}
	
	stream->flags = flags;
	stream->data = client;
	
	return -1;
}
