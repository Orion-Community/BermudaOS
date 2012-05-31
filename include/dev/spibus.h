/*
 *  BermudaOS - Serial Peripheral Interface Bus
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

//! \file include/dev/spibus.h

#ifndef __SPIBUS_H_
#define __SPIBUS_H_

#include <bermuda.h>
#include <lib/binary.h>

/**
 * \typedef SPIBUS
 * \brief SPI bus type.
 * \see _spibus
 */
typedef struct _spibus SPIBUS;

/**
 * \typedef SPICTRL
 * \brief SPI bus type.
 * \see _spictrl
 */
typedef struct _spictrl SPICTRL;

/**
 * \def BermudaSpiDevSelect
 * \brief Shortcut to select a chip using the given select line.
 * \note It will not actualy select the chip, it will set the select line. The
 *       actual select will be done by the SPI driver on reads and writes.
 * \warning The device has to be allocated before using the define!
 */
#define BermudaSpiDevSelect(dev, pin) \
{ \
        ((SPIBUS*)dev->data)->cs = pin; \
}

#define BermudaSpiDevDeselect(dev) \
{ \
	(((SPIBUS*)dev->data)->ctrl)->deselect((SPIBUS*)dev->data); \
}

#define BermudaSpiDevSetMode(dev, mode) \
{ \
	((SPICTRL*)((SPIBUS*)(dev->data))->ctrl)->set_mode((SPIBUS*)dev->data, mode); \
}

#define BermudaSpiDevSetRate(dev, rate) \
{ \
	((SPICTRL*)((SPIBUS*)(dev->data))->ctrl)->set_rate((SPIBUS*)dev->data, rate); \
}

#ifndef BERMUDA_SPI_TMO
/**
 * \def BERMUDA_SPI_TMO
 * \brief SPI time-out time in milliseconds.
 * \note Can be configured.
 */
#define BERMUDA_SPI_TMO 200
#endif

/**
 * \brief SPI mode 0.
 *
 * SCK is low on idle and data is sampled on the leading edge.
 */
#define BERMUDA_SPI_MODE0 B0

/**
 * \brief SPI mode 1.
 *
 * SCK is low on idle and data is sampled on the trailing edge.
 */
#define BERMUDA_SPI_MODE1 B1

/**
 * \brief SPI mode 2.
 *
 * SCK is high on idle and data is sampled on the leading edge.
 */
#define BERMUDA_SPI_MODE2 B10

/**
 * \brief SPI mode 0.
 *
 * SCK is low on high and data is sampled on the trailing edge.
 */
#define BERMUDA_SPI_MODE3 B11

/**
 * \def BERMUDA_SPI_MODE_UPDATE
 * \brief SPI hardware update flag.
 */
#define BERMUDA_SPI_MODE_UPDATE BIT(15)

/**
 * \def BERMUDA_SPI_RATE_UPDATE
 * \brief SPI rate update flag.
 */
#define BERMUDA_SPI_RATE_UPDATE BIT(14)

/**
 * \def BERMUDA_SPI_RATE2X
 * \brief Rate X2 hardware configuration.
 */
#define BERMUDA_SPI_RATE2X BIT(13)

// priv defs
#define BERMUDA_SPI_X2_SHIFT 11

/**
 * \struct _spibus
 * \brief SPI bus structure.
 */
struct _spibus
{
        void *queue; //!< Transfer waiting queue.
        SPICTRL *ctrl; //!< SPI bus controller \see _spibus
        void *io; //!< SPI interface control */
        volatile void *rx; //!< Used for internal receive buffering.
		volatile void *tx; //!< Used for internal transmit buffering.
        uint16_t mode; //!< SPI mode select.
        uint32_t rate; //!< SPI rate select.
        unsigned char cs; //!< Chip select pin.
};

/**
 * \struct _spictrl
 * \brief SPI bus structure.
 * 
 * SPI bus control.
 */
struct _spictrl
{        
        /**
         * \brief Transfer data.
         * \param bus SPI bus.
         * \param tx Transmit buffer.
         * \param rx Receive buffer.
         * \param len Data length.
         * \param tmo Bus timeout.
         * 
         * Transfer data over the SPI bus from the transmit buffer, while receiving
         * data in the receive buffer.
         */
        int  (*transfer)(SPIBUS* bus, const void* tx, void* rx, unsigned int len, 
			             unsigned int tmo);
        
        /**
         * \brief Transfer the SPI TX buffer.
         * \param bus SPI bus.
         * \warning Call this before reading.
         * 
         * Internal buffers will be flushed to the SPI interface. If you want to
         * read certain data back, make sure to flush the interface before
         * reading information back.
         */
        int (*flush)(SPIBUS *bus);
              
        /**
         * \brief Set data mode.
         * \param bus SPI bus.
         * \param mode Mode to set.
         * 
         * Set the given mode to this bus.
         */
        void (*set_mode)   (SPIBUS* bus, unsigned char mode);
        
        /**
         * \brief Set the clock rate.
         * \param bus SPI bus.
         * \param rate Rate in Hertz.
         * 
         * Set the given clock rate in the spi bus.
         */
        void (*set_rate)   (SPIBUS* bus, uint32_t rate);

        /**
         * \brief Change the chip select before transfer.
         * \param bus SPI bus to change chip select for.
         * \warning The bus has to be allocated before selecting it.
         */
        void (*select)(SPIBUS *bus);
        
        /**
         * \brief Deselct a chip.
         * \param bus SPI bus to deselect.
         * \warning The bus has to be allocated.
         * \note When the deselect signal is given, it is save to remove the bus
         *       mutex.
         * \see _device::alloc
         */
        void (*deselect)(SPIBUS *bus);
};

#ifdef __cplusplus
extern "C" {
#endif

extern int BermudaSPIWrite(VFILE *file, const void *tx, size_t len);
extern int BermudaSPIRead(VFILE *file, void *rx, size_t len);
extern int BermudaSPIFlush(VFILE *file);
extern uint32_t BermudaSpiRateToPrescaler(uint32_t clock, uint32_t rate, unsigned int max);
#ifdef __cplusplus
}
#endif

#endif /* __SPIBUS_H_ */
