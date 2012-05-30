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
#include <avr/interrupt.h>

#ifdef __EVENTS__
THREAD *BermudaSPI0TransferQueue = SIGNALED;
THREAD *BermudaSPI0Mutex = SIGNALED;
#endif

static HWSPI BermudaSPI0HardwareIO = {
        .spcr = &SPI_CTRL,
        .spsr = &SPI_STATUS,
        .spdr = &SPI_DATA,
};

static SPICTRL BermudaSpiHardwareCtrl = {
        .transfer = NULL,
        .flush    = NULL,
        .set_mode = NULL,
        .set_rate = NULL,
        .select   = &select,
        .deselect = &deselect,
};

static SPIBUS BermudaSpi0HardwareBus = {
#ifdef __EVENTS__
        .queue = (void*)&BermudaSPI0TransferQueue,
#else
        .queue = NULL,
#endif
        .ctrl  = &BermudaSpiHardwareCtrl,
        .io    = &BermudaSPI0HardwareIO,
        .mode  = BERMUDA_SPI_MODE0 | BERMUDA_SPI_MODE_UPDATE,
        .rate  = F_CPU/128,
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
	HWSPI *hwio = &BermudaSPI0HardwareIO;
        
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
	dev->mutex = (void*)&BermudaSPI0Mutex;

	// enable the spi interface

#ifdef __EVENTS__
	*(hwio->spcr) |= SPI_ENABLE | SPI_MASTER_ENABLE | SPI_IRQ_ENABLE;
#else
	*(hwio->spcr) |= SPI_ENABLE | SPI_MASTER_ENABLE
#endif
        
	return rc;
}

/**
 * \brief Select a chip.
 * \param bus SPI bus.
 * \todo Update SPI configuration.
 * \note The SPI bus needs to be set for a specific device first.
 * 
 * It will not only select the given pin, it will also update the SPI configuration,
 * if needed.
 */
PRIVATE WEAK void select(SPIBUS *bus)
{
	if(bus->mode & BERMUDA_SPI_MODE_UPDATE) {
		BermudaSpiRateToHwBits(&bus->rate, (bus->mode & BERMUDA_SPI_RATE2X) >> BERMUDA_SPI_X2_SHIFT);
		bus->mode &= ~(BERMUDA_SPI_MODE_UPDATE | BERMUDA_SPI_RATE2X);
		HWSPI *hw = (HWSPI*)bus->io;

		// config rate to hardware
		*(hw->spcr) = (*(hw->spcr) & (~B11)) | (bus->rate & B11);
		*(hw->spsr) = (*(hw->spsr) & (~B1)) | ((bus->rate & B100) >> 2);

		// set the mode
		*(hw->spcr) = (*(hw->spcr) & (~B1100)) | ((bus->mode & B11) << SPI_MODE_SHIFT);
	}
        BermudaSetPinMode(bus->cs, OUTPUT);
        BermudaDigitalPinWrite(bus->cs, LOW);
}

/**
 * \brief Deselect a chip.
 * \param bus SPI bus.
 * \note It is save to remove the bus/device mutex after the deselect signal.
 */
PRIVATE WEAK void deselect(SPIBUS *bus)
{
	BermudaDigitalPinWrite(bus->cs, HIGH);
}

/**
 * \brief Write data to the given SPI bus.
 * \param bus SPI bus to write to.
 * \param data Data to write.
 * \note If events are enabled, the system will wait inactive for an interrupt.
 *
 * The given data will be written to the SPI bus.
 */
PRIVATE WEAK unsigned char BermudaHardwareSpiWrite(SPIBUS* bus, unsigned char data)
{
	HWSPI *io = bus->io;
	*(io->spdr) = data;
#ifdef __EVENTS__
	if(BermudaEventWait((volatile THREAD**)bus->queue, bus->tmo) == -1) {
		return 0;
	}
#else
	while(!(*(io->spsr) & BIT(SPIF)));
#endif
	return *(io->spdr);
}

/**
 * \brief Converts an SPI rate to hardware configure bits.
 * \param rate_select Pointer to the desired rate.
 * \param spi2x When spi2x != 0, the SPI bus will be confiured with a 2X rate.
 */
PRIVATE WEAK void BermudaSpiRateToHwBits(unsigned long *rate_select, unsigned char spi2x)
{
	unsigned long pres = BermudaSpiRateToPrescaler(F_CPU, *rate_select, SPI_MAX_PRES);
	unsigned char hw = 0;

	switch(pres) {
	case 8:
	case 16:
		hw = B1;
		break;
		
	case 32:
		hw = B10;
		break;

	case 64:
		if(spi2x) {
			hw = B11;
		}
		else {
			hw = B10;
		}
		break;

	case 128:
		hw = B11;
		break;

	default:
		hw = 0;
		break;
	}

	hw |= spi2x;
	*rate_select = hw;
}

#ifdef __EVENTS__
SIGNAL(SPI_STC_vect)
{
	BermudaEventSignalFromISR((volatile THREAD**)(&BermudaSpi0HardwareBus)->queue);
}
#endif