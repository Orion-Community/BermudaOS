/*
 *  BermudaOS - SPI core driver
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

#ifndef __SPI_CORE_H
#define __SPI_CORE_H

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/spi.h>

__DECL
extern int spi_set_buff(struct spi_client *client, void *buff, size_t size);
extern int spi_flush_client(struct spi_client *client);
extern void spi_init_adapter(struct spi_adapter *adapter, char *name);
extern struct spi_client *spi_alloc_client(struct spi_adapter *adapter, reg8_t reg, uint8_t cs,
										   uint32_t freq);
extern void spi_init_adapter(struct spi_adapter *adapter, char *name);

/**
 * \brief Check wether the client is master or slave.
 * \param client spi_client to check.
 * \retval true when the client is master.
 * \retval false when the client is slave.
 */
static inline bool spi_client_is_master(struct spi_client *client)
{
	FILE *stream = client->stream;
	
	return ((stream->flags & SPI_MASTER) != 0);
}
__DECL_END

#endif