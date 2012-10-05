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

//! \file dev/devreg.c Device registration and administration.

#include <bermuda.h>

#include <lib/string.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/dev.h>

/**
 * \var BermudaDeviceRoot
 * \brief Root of the device list.
 * \note Managed by BermudaDeviceRegister and BermudaDeviceUnregister.
 * \see BermudaDeviceRegister
 * \see BermudaDeviceUnregister
 * 
 * List of all registered devices.
 */
static DEVICE *BermudaDeviceRoot;

/**
 * \brief Register a device.
 * \param dev Device to register.
 * 
 * Registers a device in the device administration.
 */
PUBLIC int BermudaDeviceRegister(DEVICE *dev, void *ioctl)
{
	int rc = -1;
	if(NULL == BermudaDeviceLoopup(dev->name)) {
		dev->next = BermudaDeviceRoot;
		BermudaDeviceRoot = dev;
		dev->ioctl = ioctl;
		dev->alloc = &BermudaDeviceAlloc;
		dev->release = &BermudaDeviceRelease;
		rc = 0;
	}
	return rc;
}

/**
 * \brief Unregister a device.
 * \param dev Device to unregister.
 * \note It might be wise to flush the I/O file associated with this device. If
 *       you do so, you are sure that all data which was ready to be written to
 *       the device is written to the device.
 * \note This function does not free the memory the device (driver) has in use.
 * \return 0 on success, -1 otherwise.
 * \see BermudaDeviceRoot
 * 
 * The given device, if registered, will be removed from the device list.
 */
PUBLIC int BermudaDeviceUnregister(DEVICE *dev)
{
	DEVICE *lookup = dev_open(dev), **dlp;
	int rc = -1;
        
	if(NULL != lookup) {
		dlp = &BermudaDeviceRoot;
		for(; *dlp; dlp = &(*dlp)->next) {
			if((*dlp) == dev) {
				*dlp = dev->next;
				rc = 0;
				break;
			}
		}      
	}
	return rc;
}

/**
 * \brief Lookup a device.
 * \param name Name of the device.
 * \return Found device. If no device is found, NULL is returned.
 * 
 * Searches for the given name in the device root list.
 */
PUBLIC DEVICE *BermudaDeviceLoopup(const char *name)
{
	DEVICE *carriage = BermudaDeviceRoot;
	DEVICE *ret = NULL;

	if(carriage == NULL) {
		return ret;
	}

	while(carriage) {
		if(strcmp(carriage->name, name) == 0) {
			ret = carriage;
			break;
		}
		carriage = carriage->next;
	}
	return ret;
}

/**
 * \brief Allocate the device.
 * \param dev Device to allocate.
 * \param tmo Time-out
 * \return 0 success, -1 otherwise.
 * 
 * The device will be locked for other threads. If the device is already locked,
 * it will wait for max. It waits for tmo milli seconds if the device is already
 * locked.
 */
PUBLIC int BermudaDeviceAlloc(DEVICE *dev, unsigned int tmo)
{
	int rc = -1;
	if(dev != NULL) {
#ifdef __EVENTS__
		if(BermudaEventWait((volatile THREAD**)dev->mutex, tmo) == 0) {
			rc = 0;
		}
#else
		rc = 0;
#endif
	}

	return rc;
}

/**
 * \brief Release the device lock.
 * \param dev Device to lock.
 * \return 0 on success, -1 on failure (i.e. the device wasn't locked). 
 * 
 * The device mutex will be released - the device can be used by other threads
 * again.
 */
PUBLIC int BermudaDeviceRelease(DEVICE *dev)
{
	int rc = -1;
	if(NULL != dev) {
#ifdef __EVENTS__
		if(BermudaEventSignal((volatile THREAD**)dev->mutex) == 0) {
			rc = 0;
		}
#else
		rc = 0;
#endif
	}
	return rc;
}

/**
 * \brief Check weather a device is locked or not.
 * \param dev Device to check.
 * \return 0 when not locked, 1 otherwise.
 * 
 * Checks if a device priority mutex is locked by another thread.
 */
PUBLIC int BermudaDeviceIsLocked( DEVICE *dev )
{
	int rc = 1;
	volatile void *lock;

	BermudaEnterCritical();
	lock = *((volatile void**)dev->mutex);
	BermudaExitCritical();

	if(lock == SIGNALED) {
		rc = 0;
	}
	return rc;
}

/**
 * \brief Open a device.
 * \param name Name of the device file.
 * \param tmo Device alloc time out.
 * \note This function will lock the device. It can be released
 *       using BermudaDeviceClose.
 * \see BermudaDeviceClose
 * \return 0 on success, -1 when tmo expires or when a device file named <b>name</b> is not found.
 * 
 * Open a device with a device file named <b>name</b>. If no file is found, -1 is returned.
 */
PUBLIC int BermudaDeviceOpen(const char *name, unsigned int tmo)
{
	DEVICE *dev = BermudaDeviceLoopup(name);
	int rc = -1;

	if(dev) {
		rc = dev->alloc(dev, tmo);
	}

	return rc;
}

