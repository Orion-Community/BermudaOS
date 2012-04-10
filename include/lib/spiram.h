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

#ifndef __SPIRAM_H

#include <bermuda.h>
#include <arch/spi.h>

/* SPI RAM opcodes */
#define WRSR 0x1
#define RDSR 0x5

// data opcodes
#define RDDA 0x3
#define WRDA 0x2

#define HOLD 0x1

typedef enum
{
        SPI_RAM_BYTE,
        SPI_RAM_PAGE,
        SPI_RAM_BUF,
} spiram_t;

__DECL
extern void BermudaSpiRamInit();
extern void BermudaSpiRamWriteByte(unsigned int address, unsigned char byte);
extern void BermudaSpiRamSetMode(spiram_t mode);
extern unsigned char BermudaSpiRamReadByte(unsigned int address);
extern void BermudaSpiRamEnable();
extern void BermudaSpiRamDisable();
__DECL_END

#endif /* __SPIRAM_H */