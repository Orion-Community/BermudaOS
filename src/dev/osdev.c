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

#include <string.h>
#include <bermuda.h>

#include <dev/dev.h>

static DEVICE *BermudaDeviceRoot;

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
