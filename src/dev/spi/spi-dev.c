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
	
	return rc;
}

/**
 * \brief Close the SPI stream.
 * \param stream Stream to close.
 * \see close
 * 
 * Do not call directly, use close instead.
 */
PUBLIC int spidev_close(FILE *stream)
{
	struct spi_client *client;
	int rc;
	
	client = (struct spi_client*)stream->data;
	if(stream) {
		free(stream);
		rc = -DEV_OK;
	} else {
		rc = -DEV_NULL;
	}
	return rc;
}

/**
 * \brief Set the transfer buffer for the upcoming SPI transmission.
 * \param stream I/O stream.
 * \param tx Transmit buffer.
 * \param size Length op \p tx.
 * \see write
 * 
 * Do not call directly, use write instead.
 */
PUBLIC int spidev_write(FILE *stream, const void *tx, size_t size)
{
	return -1;
}
