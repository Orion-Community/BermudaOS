/*
 *  BermudaOS - Device administration
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

//! \file src/dev/twidev.c TWI front-end API.

#include <bermuda.h>

#include <arch/types.h>

#include <dev/twif.h>
#include <dev/dev.h>
#include <dev/twidev.h>

#include <fs/vfile.h>

/**
 * \brief Initialize the TWI device.
 * \param bus Backend TWI structure.
 * \param name Name of the device.
 * \note The name of the device should be unique.
 */
PUBLIC DEVICE *BermudaTwiDevInit(TWIBUS *bus, char *name)
{
	DEVICE *dev = BermudaHeapAlloc(sizeof(DEVICE));
	VFILE *file = BermudaHeapAlloc(sizeof(VFILE));
	dev->name = name;
	BermudaDeviceRegister(dev, bus->io.hwio);
	
	dev->io = file;
	dev->data = bus;
        dev->mutex = bus->mutex;
	
	file->write = &BermudaTwiDevWrite;
	file->read = &BermudaTwiDevRead;
	file->flush = NULL;
	file->close = NULL;
	file->data = dev;
	
	return dev;
}

/**
 * \brief Compose a TWI message based on the given parameters.
 * \param tx Transmit buffer.
 * \param txlen Transmit buffer length.
 * \param rx Receive buffer.
 * \param rxlen Receive buffer length.
 * \param sla Slave address to address.
 * \param scl SCL frequency.
 * \param tmo Maximum waiting time-out.
 * \param call_back Used in slave transmissions. This function will be called when
 *                  a slave receive is done.
 * \return The created TWIMSG structure.
 */
PUBLIC TWIMSG *BermudaTwiMsgCompose(tx, txlen, rx, rxlen, sla, scl, tmo, call_back)
const void *tx;
size_t txlen;
void *rx;
size_t rxlen;
unsigned char sla;
uint32_t scl;
unsigned int tmo;
twi_call_back_t call_back;
{
	TWIMSG *msg = BermudaHeapAlloc(sizeof(*msg));
	msg->tx_buff = tx;
	msg->tx_length = txlen;
	msg->rx_buff = rx;
	msg->rx_length = rxlen;
	msg->sla = sla;
	msg->scl_freq = scl;
	msg->tmo = tmo;
	msg->call_back = call_back;
	
	return msg;
}

/**
 * \brief Destroy the TWI message.
 * \param msg Message to destroy.
 * 
 * The given message will be free'd but not the buffers in the message.
 */
PUBLIC void BermudaTwiMsgDestroy(TWIMSG *msg)
{
	if(msg) {
		BermudaHeapFree(msg);
	}
}

/**
 * \brief Do a master transaction which writes (<b>or reads!</b>) to/from
 *        a given TWI device.
 * \param file I/O file.
 * \param tx Pointer to a TWIMSG.
 * \param size Size of tx. Should always equal <i>sizeof(TWIMSG)</i>.
 */
PUBLIC int BermudaTwiDevWrite(VFILE *file, const void *tx, size_t size)
{
        int rc = -1;
	TWIMSG *msg = (TWIMSG*)tx;
	DEVICE *dev = (DEVICE*)(file->data);
	TWIBUS *bus = (TWIBUS*)(dev->data);
#ifdef __EVENTS__
	if(dev->alloc(file->data, msg->tmo) == -1) {
		goto out;
	}
#endif
	rc = bus->twif->transfer(bus, msg->tx_buff, msg->tx_length, msg->rx_buff, msg->rx_length,
							msg->sla, msg->scl_freq, msg->tmo);
#ifdef __EVENTS__
out:
	dev->release(file->data);
#endif
	return rc;
}

/**
 * \brief Listen as a slave and respond if there is a slave request for our
 *        device.
 * \param file I/O file.
 * \param rx Pointer to a TWIMSG.
 * \param size Size of rx. Should always equal <i>sizeof(TWIMSG)</i>.
 */
PUBLIC int BermudaTwiDevRead(VFILE *file, void *rx, size_t size)
{
	int rc = -1; size_t num = 0;
	TWIMSG *msg = (TWIMSG*)rx;
	TWIBUS *bus = ((DEVICE*)(file->data))->data;
	
	if((rc = bus->twif->listen(bus, &num, msg->rx_buff, msg->rx_length, 
							   msg->tmo)) == 0) {
		if(msg->call_back) {
			msg->call_back(msg);
		}
		bus->twif->respond(bus, msg->tx_buff, msg->tx_length, msg->tmo);
	}
	return (int)num;
}
