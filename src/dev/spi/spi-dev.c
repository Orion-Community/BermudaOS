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
#include <dev/spi-core.h>
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
	stream->write = &spidev_write;
	stream->read = &spidev_read;
	stream->close = &spidev_close;
	
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
// 	struct spi_client *client;
	int rc;
	
// 	client = (struct spi_client*)stream->data;
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
	int rc;
	struct spi_client *client = stream->data;
	
	if(!size) {
		rc = -DEV_NULL;
	} else {
		spi_set_buff(client, (void*)tx, size, SPI_TX);
		rc = -DEV_OK;
	}
	return rc;
}

/**
 * \brief Setup the read buffer for an upcoming SPI transmission.
 * \param stream I/O stream.
 * \param rx Read buffer.
 * \param size Length of \p rx.
 * \see read
 */
PUBLIC int spidev_read(FILE *stream, void *rx, size_t size)
{
	int rc;
	struct spi_client *client = stream->data;
	
	if(!size) {
		rc = -DEV_NULL;
	} else {
		spi_set_buff(client, rx, size, SPI_RX);
		rc = -DEV_OK;
	}
	
	return rc;
}
