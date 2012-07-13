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

/** \file src/lib/spiram.c 23KXXX library. */
#if (defined(__SPI__) && defined(__SPIRAM__)) || defined(__DOXYGEN__)

#include <dev/spibus.h>

#include <arch/io.h>

#include <lib/spiram.h>
#include <sys/thread.h>

/**
 * \var ram_select
 * \brief Currently select chip pin.
 */
static unsigned char ram_select = 0;

static SPIBUS *ram_bus = NULL;

/**
 * \brief Initialise the SPI ram.
 * \todo Add support for multiple SPI RAM chips.
 * 
 * Initialise the SPI communication to the SPI SRAM chip.
 */
PUBLIC void BermudaSpiRamInit(SPIBUS *bus, unsigned char cs)
{
	ram_select = cs;
	ram_bus = bus;
}

/**
 * \brief Change the chip select.
 * \param pin New chip select pin.
 * \warning Waits in a potential forever loop until the device is unlocked.
 */
PUBLIC void BermudaSpiRamSetChipSelect(uint8_t pin)
{
	ram_select = pin;
}

/**
 * \brief Write a byte.
 * \brief address Address on the SPI chip to write to.
 * \brief byte Byte to write.
 * \warning Waits in a potential forever loop until the device is unlocked.
 * 
 * Writes a given byte to the given address on the SPI chip.
 */
PUBLIC int BermudaSpiRamWriteByte(const uint16_t address, unsigned char byte)
{
	uint8_t write_seq[] = {
		WRDA, (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF), byte,
	};

	while(BermudaSpiSetSelectPinSafe(ram_bus, ram_select) == -1) {
		BermudaThreadYield();
	}
	BermudaSpiRamSetMode(SPI_RAM_BYTE);
	return BermudaSPIWrite(ram_bus, (const void*)write_seq, 
						   BERMUDA_SPIRAM_WRITE_BYTE_SEQ_LEN);
}

/**
 * \brief Read a byte.
 * \param address Address on the chip to read from.
 * \warning Waits in a potential forever loop until the device is unlocked.
 * 
 * Reads a byte from the SPI chip from the given address.
 */
PUBLIC uint8_t BermudaSpiRamReadByte(unsigned int address)
{
	uint8_t read_seq[] = {
		RDDA, (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF), 0xFF,
	};

	while(BermudaSpiSetSelectPinSafe(ram_bus, ram_select) == -1) {
		BermudaThreadYield();
	}
	BermudaSpiRamSetMode(SPI_RAM_BYTE);
	BermudaSPIWrite(ram_bus, (const void*)read_seq, 
					BERMUDA_SPIRAM_READ_BYTE_SEQ_LEN);
	return read_seq[3];
}

/**
 * \brief Change the SPI RAM mode.
 * \param mode New mode.
 * \note Currently only SPI_RAM_BYTE is supported.
 * \see spiram_t
 */
PUBLIC void BermudaSpiRamSetMode(spiram_t mode)
{
	unsigned char buff[2];
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

				while(BermudaSpiSetSelectPinSafe(ram_bus, ram_select) == -1) {
					BermudaThreadYield();
				}
                BermudaSPIWrite(ram_bus, buff, 2);
        }
}
#endif /* __SPI__ && __SPIRAM__*/
