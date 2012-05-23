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

#include <string.h>
#include <bermuda.h>

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
        if(NULL == BermudaDeviceLoopup(dev->name))
        {
                dev->next = BermudaDeviceRoot;
                BermudaDeviceRoot = dev;
                dev->ioctl = ioctl;
                if(dev)
                        dev->init(dev);
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
 */
PUBLIC int BermudaDeviceUnregister(DEVICE *dev)
{
        DEVICE *lookup = dev_open(dev), **dlp;
        int rc = -1;
        
        if(NULL != lookup)
        {
                dlp = &BermudaDeviceRoot;
                for(; *dlp; dlp = &(*dlp)->next)
                {
                        if((*dlp) == dev)
                        {
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
        DEVICE *ret = BermudaDeviceRoot;
        
        while(ret)
        {
                if(strcmp(ret->name, name) == 0)
                        break;
        }
        return ret;
}
