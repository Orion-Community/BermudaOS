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

/**
 * \def dev_open
 * \brief Open a device.
 * \param name Name of the device.
 * \return Opened device. If no device was openend, NULL is returned.
 * 
 * This define will look for files with the given name. If a file is found,
 * the associated device will be returned.
 */
#define dev_open(name)             \
{                                  \
        BermudaDeviceLoopup(name); \
}

#define dev_write(dev, tx, len) dev->io->write(dev->io, tx, len)
#define dev_read (dev, rx, len) dev->io->read (dev->io, rx, len)
#define dev_flush(dev)          dev->io->flush(dev->io)
#define dev_close(dev)          dev->io->close(dev->io)

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
        void *ioctl; //!< Device I/O control block.
        
        /**
         * \brief Init this device.
         * \param dev 'This' device. A pointer to itself.
         * \note Called by BermudaDeviceRegister
         * \see BermudaDeviceRegister
         * 
         * Initialise the device driver and device structure.
         */
        void (*init)(struct _device *dev);
};

/**
 * \typedef DEVICE
 * \brief Device type.
 */
typedef struct _device DEVICE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Register a device.
 * \param dev Device to register.
 * 
 * Registers a device in the device administration.
 */
extern int BermudaDeviceRegister(DEVICE *dev, void *ioctl);

/**
 * \brief Unregister a device.
 * \param dev Device to unregister.
 * \note It might be wise to flush the I/O file associated witht this device. If
 *       you do so, you are sure that all data which was ready to be written to
 *       the device is written to the device.
 */
extern void BermudaDeviceUnregister(DEVICE *dev);

/**
 * \brief Lookup a device.
 * \param name Name of the device.
 * \return Found device. If no device is found, NULL is returned.
 * 
 * Searches for the given name in the device root list.
 */
extern DEVICE *BermudaDeviceLoopup(const char *name);
#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */