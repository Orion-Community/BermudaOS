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

// TWI IO defines

/**
 * \def TW_ENABLE
 * \brief Enables the TWI interface.
 * 
 * The following bits will be set: \n
 * * TWEN \n
 * * TWIE \n
 * * TWEA \n
 */
#define TW_ENABLE (BIT(TWIE) | BIT(TWEN) | BIT(TWEA))

/**
 * \def TW_DISABLE
 * \brief Bit mask to disable the TWI interface.
 * \warning Not a set value like TW_ENABLE! It is a mask.
 * 
 * Results in 0xFA or (11111010B).
 */
#define TW_DISABLE_MASK (~((~BIT(TWIE)) ^ (~BIT(TWEN))))

/**
 * \def TW_ACK
 * \brief Enable the TWI interface + interrupt.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN \n
 * * TWIE \n
 * * TWEA \n
 * * TWINT \n
 */
#define TW_ACK (BIT(TWIE) | BIT(TWEN) | BIT(TWEA) | BIT(TWINT))

/**
 * \def TW_STOP
 * \brief Enable the TWI interface and sent a start condition.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN \n
 * * TWIEN \n
 * * TWSTO \n
 * * TWEA \n
 */
#define TW_STOP (TW_ACK | BIT(TWSTO))

/**
 * \def TW_START
 * \brief Enable the TWI interface and sent a start condition.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN \n
 * * TWIEN \n
 * * TWSTA \n
 * * TWEA \n
 */
#define TW_START (TW_ACK | BIT(TWSTA))

/**
 * \def TW_RELEASE
 * \brief Release the TWI interface.
 * \note Please note that this will <b>disable</b> the TWI hardware interrupt.
 * 
 * The following bits will be set: \n
 * * TWEN \n
 * * TWEA \n
 * * TWINT \n
 */
#define TW_RELEASE (TW_ACK & (~BIT(TWIE)))

#define TW_LISTEN TW_ACK //!< Listen on the interface for incoming slave requests.

#define TW_BLOCK_MASK ~(BIT(TWINT) | BIT(TWIE)) //!< Blocks the interface.

/**
 * \def TW_NACK
 * \brief Enable the interface, but disable ACKing.
 * 
 * The following bits will be enabled: \n
 * * TWEN \n
 * * TWIE \n
 * * TWINT \n
 */
#define TW_NACK (TW_ACK & (~BIT(TWEA)))

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
struct _twi_hw {
	volatile reg8_t twbr;  //!< TWI bit rate control register.
	volatile reg8_t twcr;  //!< TWI control register.
	volatile reg8_t twsr;  //!< TWI status register.
	volatile reg8_t twdr;  //!< TWI data register.
	volatile reg8_t twar;  //!< TWI (slave) address register
	volatile reg8_t twamr; //!< TWI (slave) address mask register.
};

/**
 * \typedef TWIHW
 * \brief Type definition of the HW I/O structure.
 */
typedef struct _twi_hw TWIHW;

__DECL
extern void BermudaTwi0Init(unsigned char sla);
extern int BermudaTwiMasterTransfer(TWIBUS *twi, const void *tx, unsigned int txlen,  
							  void *rx, unsigned int rxlen, unsigned char sla,
							  uint32_t frq, unsigned int tmo);
extern unsigned char BermudaTwiCalcTWBR(uint32_t freq, unsigned char pres);
extern unsigned char BermudaTwiCalcPres(uint32_t pres);

PRIVATE WEAK void BermudaTwInit(TWIBUS *twi, const void *tx, unsigned int txlen, 
								void *rx, unsigned int rxlen, unsigned char sla, 
								uint32_t frq);
PRIVATE WEAK int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);
__DECL_END

extern TWIBUS *TWI0;

#endif
