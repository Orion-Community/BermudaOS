/*
 *  BermudaOS - Device drivers
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

/** \file dev/dev.h */

#ifndef __DEV_H__
#define __DEV_H__

#include <bermuda.h>

#include <fs/vfile.h>
#include <sys/events/event.h>

/**
 * \def dev_open
 * \brief Open a device.
 * \param name Name of the device.
 * \return Opened device. If no device was openend, NULL is returned.
 * 
 * This define will look for files with the given name. If a file is found,
 * the associated device will be returned.
 */
#define dev_open(name) BermudaDeviceLoopup((const char*)name)
/**
 * \def dev_write
 * \brief Write to a device.
 * \param dev Device to write to.
 * \param tx Transmit buffer.
 * \param len Length of tx.
 * 
 * This define will write to the I/O file associated with the given device.
 */
#define dev_write(dev, tx, len) ((dev)->io->write(dev->io, tx, len))

/**
 * \def dev_read
 * \brief Read from a device.
 * \param dev Device to read from.
 * \param rx Receive buffer.
 * \param len Length of rx.
 * 
 * Writes to the I/O file associated with the given device.
 */
#define dev_read(dev, rx, len) ((dev)->io->read (dev->io, rx, len))

/**
 * \def dev_flush
 * \brief Flush a device.
 * \param dev Device to flush.
 * \note Most device drivers reset their internal buffers when the flush function
 *       is called. Be you have read all bytes from the receive buffer before
 *       flushing the device.
 * 
 * Flush the I/O file associated with the given device.
 */
#define dev_flush(dev)          dev->io->flush(dev->io)

/**
 * \def dev_close
 * \brief Close a device.
 * \param dev Device to close.
 * \note In most cases, before the device is closed, the device will be flushed.
 * 
 * Flush the I/O file associated with the given device.
 */
#define dev_close(dev)          dev->io->close(dev->io)

/**
 * \typedef DEVICE
 * \brief Device type.
 */
typedef struct _device DEVICE;

/**
 * \struct _device
 * \brief Device information structure.
 */
struct _device
{
	struct _device *next; //!< Next pointer. Handled by device administration.
	
	/**
	 * \brief Device name.
	 * \warning <b>MUST</b> be unique.
	 */
	char *name;
	VFILE *io; //!< Virtual file I/O member.
	void *data; //!< Device specific data.
	
	void (*ctrl)(DEVICE *dev, int reg, void *data);
	void *ioctl; //!< Device I/O control block.
	void *mutex; //!< Device mutex.

	/**
	 * \brief Allocate the device.
	 * \param dev 'This' device.
	 * \param tmo Time out value in milli seconds.
	 * \return Returns 0 on success, -1 if the device could not be allocated.
	 */
	int (*alloc)(struct _device *dev, unsigned int tmo);

	/**
	 * \brief Release the device.
	 * \param dev 'This' device.
	 * \return 0 on success, -1 when the device couldn't be released.
	 */
	int (*release)(struct _device *dev);
};

#ifdef __cplusplus
extern "C" {
#endif
	
extern int BermudaDeviceRegister(DEVICE *dev, void *ioctl);
extern int BermudaDeviceUnregister(DEVICE *dev);
extern int BermudaDeviceAlloc(DEVICE *dev, unsigned int tmo);
extern int BermudaDeviceRelease(DEVICE *dev);
extern DEVICE *BermudaDeviceLoopup(const char *name);
extern int BermudaDeviceIsLocked(DEVICE *dev);
extern int BermudaDeviceOpen(const char *name, unsigned int tmo);

// inline funcs

/**
 * \brief Close a device file.
 * \param file Device file.
 * 
 * The given file will be flushed and closed.
 */
static inline int BermudaDeviceClose(VFILE *file)
{
	DEVICE *dev = file->data;
	return dev->release(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */