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
#include <arch/types.h>

/**
 * \typedef SPINODE
 * \brief SPI node type.
 * \see _spinode
 */
typedef struct _spinode SPINODE;

/**
 * \typedef SPIBUS
 * \brief SPI bus type.
 * \see _spibus
 */
typedef struct _spibus SPIBUS;

/**
 * \struct _spinode
 * \brief SPI node structure.
 */
struct _spinode
{
        unsigned char cs; //!< Chip select pin.
        SPIBUS *bus; //!< SPI bus controller \see _spibus
        void   *spi_ctr; //!< SPI interface control */
        uint32_t mode; //!< SPI mode select.
        uint32_t rate; //!< SPI rate select.
        
        /**
         * \brief Change the chip select before transfer.
         * \param node SPI node to change chip select for.
         * \param pin  MCU pin. See the package description of the used CPU.
         * \warning The bus has to be allocated before selecting it.
         */
        void (*select)(SPINODE *node, unsigned char pin);
};

/**
 * \struct _spibus
 * \brief SPI bus structure.
 * 
 * SPI bus control.
 */
struct _spibus
{
        void *iobase; //!< I/O base address.
        void *mutex; //!< Bus mutex.
        void *queue; //!< Transfer waiting queue.
        
        /**
         * \brief Initialise a node.
         * \param node Node to initialise.
         * 
         * Function pointer to the SPI bus init function.
         */
        void (*init_node)  (SPINODE* node);
        
        /**
         * \brief Transfer data.
         * \param node SPI node.
         * \param tx Transmit buffer.
         * \param rx Receive buffer.
         * \param len Data length.
         * \param tmo Bus timeout.
         * 
         * Transfer data over the SPI bus from the transmit buffer, while receiving
         * data in the receive buffer.
         */
        int  (*transfer)(SPINODE* node, const void* tx, void* rx, int len, int tmo);
        
        /**
         * \brief Lock the bus.
         * \param tmo Time-out.
         * 
         * Lock the bus. If the bus is already locked, it will wait for <b>tmo</b>
         * milli seconds to receive a release from another node holding it.
         */
        int  (*bus_alloc)  (SPINODE*, unsigned int tmo);
        
        /**
         * \brief Release the bus.
         * \param node Bus node to release.
         * 
         * The bus associated to the given node is released.
         */
        void (*bus_release)(SPINODE* node);
        
        /**
         * \brief Set data mode.
         * \param node SPI node.
         * \param mode Mode to set.
         * 
         * Set the given mode to this bus.
         */
        void (*set_mode)   (SPINODE* node, unsigned short mode);
        
        /**
         * \brief Set the clock rate.
         * \param node SPI node.
         * \param rate Rate in Hertz.
         * 
         * Set the given clock rate in the spi bus.
         */
        void (*set_rate)   (SPINODE* node, uint32_t rate)
};

#endif /* __SPIBUS_H_ */
