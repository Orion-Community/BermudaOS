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

#include <sys/mem.h>
#include <sys/events/event.h>

#include <dev/dev.h>
#include <dev/spibus.h>

/**
 * \brief Write to the SPI interface.
 * \param file SPI device I/O file.
 * \param tx Transmit buffer.
 * \param len Length of <b>tx</b>.
 * \note This function writes to the back ground buffers, not to the actual
 *       interface.
 */
PUBLIC int BermudaSPIWrite(VFILE *file, void *tx, size_t len)
{
        int rc = -1;
        SPIBUS *spi = (SPIBUS*)((DEVICE*)file->data)->data;
        
        rc = spi->ctrl->transfer(spi, tx, NULL, len, BERMUDA_SPI_TMO);
        return rc;
}

/**
 * \brief Read from the SPI device.
 * \param file I/O file.
 * \param rx Receive buffer.
 * \param len Length of <b>rx</b>.
 */
PUBLIC int BermudaSPIRead(VFILE *file, void *rx, size_t len)
{
        int rc = -1;
        SPIBUS *spi = (SPIBUS*)((DEVICE*)file->data)->data;
        spi->ctrl->select(spi);
        rc = spi->ctrl->transfer(spi, NULL, rx, len, BERMUDA_SPI_TMO);
        spi->ctrl->deselect(spi);
        return rc;
}

/**
 * \brief Flush SPI.
 * \param file I/O file.
 * 
 * Flush function for the SPI I/O file.
 */
PUBLIC int BermudaSPIFlush(VFILE *file)
{
	int rc = -1;
	SPIBUS *spi = (SPIBUS*)((DEVICE*)file->data)->data;
	spi->ctrl->select(spi);
	//rc = spi->ctrl->flush(spi);
	spi->ctrl->deselect(spi);
	return rc;
}

/**
 * \brief Convert a clock rate to a SPI prescaler.
 * \param clock Base clock speed.
 * \param rate Desired rate.
 * \param max Maximum prescaler
 * \return Clock prescaler.
 * \note Clock should be a power of two.
 *
 * Calculates the divider to get the given rate.
 */
PUBLIC uint32_t BermudaSpiRateToPrescaler(uint32_t clock, uint32_t rate, unsigned int max)
{
	uint32_t pres = clock / rate;
	if(pres > max) {
		pres = max;
	}
	return pres;
}
