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
#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/spi.h>
#include <dev/spi-core.h>

#include <lib/spiram.h>
#include <sys/thread.h>

/**
 * \brief SPIRAM SPI adapter.
 */
static struct spi_client *client;

#if 0
static void BermudaSpiRamSetMode(spiram_t mode);
#endif

/**
 * \brief Initialise the SPI ram.
 * 
 * Initialise the SPI communication to the SPI SRAM chip.
 */
PUBLIC void spiram_init(struct spi_adapter *adapter, reg8_t port, uint8_t cs)
{
	client = spi_alloc_client(adapter, port, cs, SPI_1MHZ);
}

/**
 * \brief Write a byte.
 * \brief address Address on the SPI chip to write to.
 * \brief byte Byte to write.
 * \warning Waits in a potential forever loop until the device is unlocked.
 * 
 * Writes a given byte to the given address on the SPI chip.
 */
PUBLIC int spiram_write_byte(const uint16_t address, uint8_t byte)
{
	int fd, rc = -1;
	uint8_t write_seq[] = {
		WRDA, (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF), byte,
	};

	fd = spidev_socket(client, _FDEV_SETUP_RW | SPI_MASTER);
	if(fd >= 0) {
		write(fd, (void*)write_seq, BERMUDA_SPIRAM_WRITE_BYTE_SEQ_LEN);
		rc = flush(fd);
		close(fd);
	}
	
	return rc;
}

/**
 * \brief Read a byte.
 * \param address Address on the chip to read from.
 * \warning Waits in a potential forever loop until the device is unlocked.
 * 
 * Reads a byte from the SPI chip from the given address.
 */
PUBLIC uint8_t spiram_read_byte(unsigned int address)
{
	int fd;
	uint8_t read_seq[] = {
		RDDA, (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF), 0xFF,
	};

	fd = spidev_socket(client, _FDEV_SETUP_RW | SPI_MASTER);
	
	if(fd >= 0) {
		write(fd, (void*)read_seq, BERMUDA_SPIRAM_READ_BYTE_SEQ_LEN);
		flush(fd);
		close(fd);
	}
	
	return read_seq[3];
}

#if 0
/**
 * \brief Change the SPI RAM mode.
 * \param mode New mode.
 * \note Currently only SPI_RAM_BYTE is supported.
 * \see spiram_t
 */
static void BermudaSpiRamSetMode(spiram_t mode)
{
	unsigned char buff[2];
	int fd;
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
		fd = spidev_socket(client, _FDEV_SETUP_RW | SPI_MASTER);
			
		if(fd >= 0) {
			write(fd, buff, 2);
			read(fd, buff, 2);
			flush(fd);
			close(fd);
		}
	}
}
#endif

