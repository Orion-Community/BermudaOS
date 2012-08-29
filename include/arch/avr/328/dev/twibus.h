/*
 *  BermudaOS - TWI bus
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

//! \file arch/avr/328/dev/twibus.h

#ifndef __TWI_BUS_H
#define __TWI_BUS_H

#include <bermuda.h>
#include <dev/twif.h>
#include <arch/avr/328/dev/twireg.h>

/**
 * \def TWI0
 * \brief TWI hardware interface 0 data structure.
 */
#define TWI0 twibus0

/**
 * \def TWI0_INIT
 * \brief Initialization routine of TWI hardware interface 0.
 * \see TWI0
 */
#define TWI0_INIT BermudaTwi0Init

/**
 * \def BermudaTwiIoData
 * \brief Get I/O data address.
 */
#define BermudaTwiIoData(bus) ((HWTWI*)bus->io.hwio)

/**
 * \def TW_TMO
 * \brief Default TWI timeout.
 * 
 * Time-out used for internal time-outs (aka event waits).
 */
#define TW_TMO 200

// prescaler defines
/**
 * \def TW_PRES_1
 * \brief Prescaler of 1. 
 * 
 * This value has no effect on the prescaler value.
 */
#define TW_PRES_1 B0

/**
 * \def TW_PRES_4
 * \brief Prescaler value 4.
 */
#define TW_PRES_4 B1

/**
 * \def TW_PRES_16
 * \brief Prescaler value 16
 */
#define TW_PRES_16 B10

/**
 * \def TW_PRES_64
 * \brief Prescaler value 64
 */
#define TW_PRES_64 B11

/**
 * \def TWI_FRQ
 * \brief Calculates the TWI frequency based on the given twbr and prescaler.
 * \param x TWBR
 * \param n The prescaler.
 */
#define TWI_FRQ(x, n) \
	(F_CPU/(16+(2*x*n)))

/**
 * \brief TWI hardware I/O.
 * 
 * Structure containing all hardware I/O registers.
 */
struct _hw_twi {
	volatile reg8_t twbr;  //!< TWI bit rate control register.
	volatile reg8_t twcr;  //!< TWI control register.
	volatile reg8_t twsr;  //!< TWI status register.
	volatile reg8_t twdr;  //!< TWI data register.
	volatile reg8_t twar;  //!< TWI (slave) address register
	volatile reg8_t twamr; //!< TWI (slave) address mask register.
	
	// generic io registers used in twi
	reg8_t io_in; //!< SCL/SDA input register.
	reg8_t io_out; //!< SLA/SDA output register.
	
	// pins
	unsigned char scl; //!< SCL pin.
	unsigned char sda; //!< SDA pin.
};

/**
 * \typedef HWTWI
 * \brief Type definition of the HW I/O structure.
 */
typedef struct _hw_twi HWTWI;

__DECL
extern void BermudaTwi0Init(unsigned char sla);
extern unsigned char BermudaTwiCalcTWBR(uint32_t freq, unsigned char pres);
extern unsigned char BermudaTwiCalcPres(uint32_t pres);
extern TWIBUS *BermudaTwiBusFactoryCreate(unsigned char sla);
extern void BermudaTwiBusFactoryDestroy(TWIBUS *bus, TWI_BUS_TYPE type);
PRIVATE WEAK int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);
__DECL_END

extern TWIBUS *TWI0;

#endif
