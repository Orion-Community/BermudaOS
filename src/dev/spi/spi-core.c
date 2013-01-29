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

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/error.h>
#include <dev/spi.h>
#include <dev/spi-core.h>

PUBLIC void spi_init_adapter(struct spi_adapter *adapter, char *name)
{
	struct device *dev = malloc(sizeof(*dev));
	
	dev->name = name;
	BermudaDeviceRegister(dev, adapter);
	
	adapter->busy = FALSE;
	adapter->features = 0;
	adapter->error = 0;
}

/**
 * \brief Set the a transmission buff.
 * \param client Client requesting the transmission.
 * \param buff Buffer to set.
 * \param size The length of \p buff.
 * \param trans_type Set to SPI_TX when \p buff is a transmit buffer. When buff is a receive buffer
 *                   \p trans_type should be set to SPI_RX.
 */
PUBLIC int spi_set_buff(struct spi_client *client, void *buff, size_t size,
				 spi_transmission_type_t trans_type)
{
	struct spi_adapter *adapter = client->adapter;
	int rc = -DEV_OK;
	
	switch(trans_type) {
		case SPI_TX:
			adapter->tx = buff;
			adapter->tx_size = size;
			break;
			
		case SPI_RX:
			adapter->rx = buff;
			adapter->rx_size = size;
			break;
			
		default:
			rc = -DEV_ERROR;
			break;
	}
	
	return rc;
}

PUBLIC int spi_flush_client(struct spi_client *client)
{
	return -1;
}
