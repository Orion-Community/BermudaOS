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
 * \brief Set the SPI chip select line.
 * \note It will not actualy select the chip, it will set the select line. The
 *       actual select will be done by the SPI driver on reads and writes.
 * \warning The device has to be allocated before using the define!
 */
#define BermudaSpiSetSelectPin(dev, pin) \
{ \
        dev->cs = pin; \
}

/**
 * \brief Deselect an SPI chip.
 * \deprecated
 */
#define BermudaSpiDevDeselect(dev) \
{ \
	dev->ctrl->deselect(dev); \
}

/**
 * \brief Set an SPI mode.
 * \deprecated Use BermudaSpiSetModeSafe instead.
 * \see BermudaSpiSetModeSafe
 */
#define BermudaSpiDevSetMode(dev, mode) \
{ \
	dev->ctrl->set_mode(dev, mode); \
}

/**
 * \brief Set an SPI SCK rate.
 * \deprecated Use BermudaSpiSetRateSafe instead.
 * \see BermudaSpiSetRateSafe
 */
#define BermudaSpiDevSetRate(dev, rate) \
{ \
	dev->ctrl->set_rate(dev, rate); \
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

/**
 * \def BERMUDA_SPI_MASTER
 * \brief Definition of the SPI master type.
 */
#define BERMUDA_SPI_MASTER 0xF

/**
 * \def BERMUDA_SPI_SLAVE
 * \brief Definition of the SPI slave type.
 */
#define BERMUDA_SPI_SLAVE  0xF0

// priv defs
#define BERMUDA_SPI_X2_SHIFT 11

typedef enum
{
	SPI_START,
	SPI_STOP,
	SPI_IDLE,
	
	SPI_ENABLE_MASTER,
	SPI_ENABLE_SLAVE,
	
	SPI_READ_DATA,
	SPI_WRITE_DATA,
} SPI_IOCTL_MODE;

/**
 * \struct _spibus
 * \brief SPI bus structure.
 */
struct _spibus
{
#ifdef __EVENTS__
	void *mutex; //!< SPI bus mutex.
	void *master_queue; //!< Transfer waiting queue.
	void *slave_queue; //!< Slave transfer waiting queue.
#elif __THREADS__
	mutex_t mutex;
	mutex_t queue;
#endif
	SPICTRL *ctrl; //!< SPI bus controller \see _spictrl
	void *io; //!< SPI interface control */
	uint16_t mode; //!< SPI mode select.
	uint32_t rate; //!< SPI rate select.
	unsigned char cs; //!< Chip select pin.
	unsigned char bus_type; //!< Master/slave selector.
	
	const unsigned char *master_tx;
	const unsigned char *master_rx;
	uptr master_len;
	uptr master_index;
	
	const unsigned char *slave_tx;
	const unsigned char *slave_rx;
	uptr slave_len;
	uptr slave_index;
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
	int  (*transfer)(SPIBUS* bus, const const uint8_t *tx, uint8_t *rx, uptr len, 
						unsigned int tmo);
			
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
	
	/**
	 * \brief SPI interrupt service handler.
	 * \param bus SPI bus to service.
	 */
	void (*isr)(SPIBUS *bus);
	
	void (*io)(SPIBUS *bus, SPI_IOCTL_MODE mode, void *data);
};

#ifdef __cplusplus
extern "C" {
#endif

extern int BermudaSPIWrite(SPIBUS *bus, const void *tx, size_t len);
extern uint32_t BermudaSpiRateToPrescaler(uint32_t clock, uint32_t rate, unsigned int max);
extern void BermudaSpiISR(SPIBUS *bus);

// inline funcs
 
/**
 * \brief Safely try to cs.
 * \param dev Device to set the cs pin for.
 * \param pin Chip select pin.
 * \see BermudaSpiSetSelectPin
 * \return 0 when set successfully, -1 otherwise.
 * 
 * Safely try to set the chip select pin in the SPIBUS structure.
 */
static inline int BermudaSpiSetSelectPinSafe(SPIBUS *spidev, uint8_t cs)
{
	BermudaSpiSetSelectPin(spidev, cs);
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __SPIBUS_H_ */
