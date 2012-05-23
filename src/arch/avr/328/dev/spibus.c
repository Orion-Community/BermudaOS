/*
 *  BermudaOS - SPI bus driver
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

#include <bermuda.h>

#include <fs/vfile.h>

#include <sys/thread.h>
#include <sys/mem.h>
#include <sys/events/event.h>

#include <dev/dev.h>
#include <dev/spibus.h>

#include <arch/avr/io.h>
#include <arch/avr/328/dev/spireg.h>
#include <arch/avr/328/dev/spibus.h>

#ifdef __EVENTS__
THREAD *BermudaSPI0TransferQueue = NULL;
#endif

static HWSPI BermudaSPI0HardwareIO = {
        .spcr = &SPI_CTRL,
        .spsr = &SPI_STATUS,
        .spdr = &SPI_DATA,
};

static SPICTRL BermudaSpi0HardwareCtrl = {
        .transfer = NULL,
        .flush    = NULL,
        .set_mode = NULL,
        .set_rate = NULL,
        .select   = NULL,
        .deselect = NULL,
};

static SPIBUS BermudaSpi0HardwareBus = {
#ifdef __EVENTS__
        .queue = (void*)&BermudaSPI0TransferQueue,
#else
        .queue = NULL,
#endif
        .ctrl  = &BermudaSpi0HardwareCtrl,
        .io    = &BermudaSPI0HardwareIO,
        .mode  = 0,
        .rate  = 0,
        .cs    = 0,
};

/**
 * \brief Initialize hardware SPI bus 0.
 * \param dev Device to initialize.
 * \return 0 On success, -1 otherwise (allocation of memory failed).
 * \see _device
 * \see SPI0
 * \see _vfile
 * 
 * When this function returns 0, the device is ready to use.
 */
PUBLIC int BermudaSPI0HardwareInit(DEVICE *dev)
{
        int rc = -1;
        
        if((dev->io = BermudaHeapAlloc(sizeof(*dev->io))) == NULL)
                return rc;
        rc = 0;
        
        // initialize the file
        dev->io->write = &BermudaSPIWrite;
        dev->io->read = &BermudaSPIRead;
        dev->io->flush = &BermudaSPIFlush;
        dev->io->close = NULL;
        dev->io->mode = 0;
        dev->io->data = (void*)dev;
        
        dev->data = &BermudaSpi0HardwareBus;
        
        return rc;
}
