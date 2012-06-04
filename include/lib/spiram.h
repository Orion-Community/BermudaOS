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

//! \file include/lib/spiram.h 23KXXX definitions.

#ifndef __SPIRAM_H

#include <bermuda.h>
#include <arch/spi.h>

/* SPI RAM opcodes */
/**
 * \def WRSR
 * \brief Write status register opcode.
 */
#define WRSR 0x1

/**
 * \def RDSR
 * \brief Read status register opcode.
 */
#define RDSR 0x5

// data opcodes
/**
 * \def RDDA
 * \brief Read data opcode.
 */
#define RDDA 0x3

/**
 * \def WRDA
 * \brief Write data opcode.
 */
#define WRDA 0x2

/**
 * \def HOLD
 * \brief Hold pin disable bit in the status register.
 */
#define HOLD 0x1

// general defines
/**
 * \def BERMUDA_SPIRAM_WRITE_BYTE_SEQ_LEN
 * \brief Length of the write byte command.
 */
#define BERMUDA_SPIRAM_WRITE_BYTE_SEQ_LEN 4

/**
 * \def BERMUDA_SPIRAM_READ_BYTE_SEQ_LEN
 * \brief Length of the read byte command.
 */
#define BERMUDA_SPIRAM_READ_BYTE_SEQ_LEN  4

/**
 * \typedef spiram_t
 * \brief SPIRAM mode.
 * 
 * Mode selection in the SPIRAM status register.
 */
typedef enum
{
        SPI_RAM_BYTE, //!< Read SPI ram byte by byte.
        SPI_RAM_PAGE, //!< Read 32 bytes, in a row, at once.
        SPI_RAM_BUF,  //!< Read an array of bytes in a row at once.
} spiram_t;

__DECL
extern void BermudaSpiRamInit(const char *dev, unsigned char cs);
extern void BermudaSpiRamSetChipSelect(uint8_t pin);
extern int BermudaSpiRamWriteByte(const unsigned int address, unsigned char byte);
extern void BermudaSpiRamSetMode(spiram_t mode);
extern uint8_t BermudaSpiRamReadByte(unsigned int address);
__DECL_END

#endif /* __SPIRAM_H */