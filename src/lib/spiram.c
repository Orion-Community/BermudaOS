/*
 *  BermudaOS - SpiRam library
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

/** \file spiram.c */
#if (defined(__SPI__) && defined(__SPIRAM__)) || defined(__DOXYGEN__)

#include <dev/dev.h>
#include <dev/spibus.h>

#include <arch/io.h>
#include <lib/spiram.h>

static unsigned char ram_select = 0;
PRIVATE WEAK unsigned char _current_mode = 0xff;

/**
 * \fn BermudaSpiRamInit()
 * \brief Initialise the SPI ram.
 * \todo Use high level SPI interface.
 * \todo Add init check.
 * 
 * Initialise the SPI communication to the SPI SRAM chip.
 */
PUBLIC void BermudaSpiRamInit()
{

}

/**
 * \brief Select an SPI ram chip using the chip select line.
 * \param cs Chip select pin.
 */
PUBLIC inline void BermudaSpiRamChipSelect(unsigned char cs)
{
	ram_select = cs;
}

PUBLIC int BermudaSpiRamWriteByte(const uint16_t address, unsigned char byte)
{
	uint8_t write_seq[] = {
		WRDA, (address >> 8) & 0xFF, address & 0xFF, byte,
	};
	DEVICE *spidev = dev_open("SPI0");

	if(spidev->alloc(spidev, BERMUDA_SPI_TMO) == -1) {
		return -1;
	}

	BermudaSpiRamSetMode(SPI_RAM_BYTE);
	BermudaSpiDevSelect(spidev, ram_select);
	dev_write(spidev, (const void*)write_seq, BERMUDA_SPIRAM_WRITE_BYTE_SEQ_LEN);
	BermudaSpiDevDeselect(spidev);
	spidev->release(spidev);

	return 0;
}

PUBLIC void BermudaSpiRamSetMode(spiram_t mode)
{
	DEVICE *spidev = dev_open("SPI0");
	unsigned char *buff = BermudaHeapAlloc(2);
        if(mode <= SPI_RAM_BUF)
        {
                unsigned char status = HOLD;
                
                switch(mode)
                {
                        case SPI_RAM_BYTE:
                                break;
                                
                        case SPI_RAM_PAGE:
                                status |= 0x80;
                                break;
                                
                        case SPI_RAM_BUF:
                                status |= 0x40;
                                break;

                        default:
                                status = 0;
                                break;
                }
                
				buff[0] = WRSR; buff[1] = status;

                BermudaSpiDevSelect(spidev, ram_select);
                dev_write(spidev, buff, 2);
				BermudaSpiDevDeselect(spidev);
        }

		BermudaHeapFree(buff);
}
#endif /* __SPI__ && __SPIRAM__*/
