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

/* static functions */
static inline int spi_unlock_adapter(struct spi_adapter *adapter, bool master);
static inline int spi_lock_adapter(struct spi_adapter *adapter, bool master);

/**
 * \brief Initialize a spi_adapter structure.
 */
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
	int rc = -DEV_OK;
	
	if(client->length < size) {
		client->length = size;
	}
	
	switch(trans_type) {
		case SPI_TX:
			client->tx = buff;
			break;
			
		case SPI_RX:
			client->rx = buff;
			break;
			
		default:
			rc = -DEV_ERROR;
			break;
	}
	
	return rc;
}

/**
 * \brief Flush the client to the bus.
 * \param client Client to flush.
 * \retval 0 on success.
 * \return Error code. Non zero if an error has occurred.
 */
PUBLIC int spi_flush_client(struct spi_client *client)
{
	struct spi_adapter *adapter = client->adapter;
	struct spi_shared_info info;
	
	int rc;
	if((rc = spi_lock_adapter(adapter, spi_client_is_master(client)) ) == 0) {
		adapter->tx = client->tx;
		adapter->rx = client->rx;
		adapter->length = client->length;
		info.cs = client->cs;
		info.freq = client->freq;
		info.cspin = client->cspin;
		
		rc = adapter->xfer(adapter, &info);
		adapter->length = 0;
		spi_unlock_adapter(adapter, spi_client_is_master(client));
	}
	return rc;
}

/**
 * \brief Lock the adapter.
 * \param adapter SPI bus adapter.
 * \param master Set to true for master transfers, false for slave transmissions.
 * \retval 0 on success.
 * \retval -1 on failure.
 * 
 * The adapter will only be blocked if a master transfer is done.
 */
static inline int spi_lock_adapter(struct spi_adapter *adapter, bool master)
{
	struct device *dev = adapter->dev;
	int rc;
	
	if(master) {
		rc = dev->alloc(dev, 500);
	} else {
		rc = 0;
	}
	
	return rc;
}

/**
 * \brief Unlock the adpater.
 * \param adapter SPI bus adapter.
 * \param master Set to true for master transfers, false for slave transmissions.
 * \retval 0 on success.
 * \retval -1 on failure.
 * 
 * Unlock the bus adapter.
 */
static inline int spi_unlock_adapter(struct spi_adapter *adapter, bool master)
{
	struct device *dev = adapter->dev;
	int rc;
	
	if(master) {
		rc = dev->release(dev);
	} else {
		rc = 0;
	}
	
	return rc;
}
