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

// Master transmitter status bytes

/**
 * \brief Start has been sent.
 */
#define TWI_START 0x8

/**
 * \brief Repeated start has been sent.
 */
#define TWI_REP_START 0x10

/**
 * \brief Slave address has been sent and ACKed.
 */
#define TWI_MT_SLA_ACK 0x18

/**
 * \brief Slave address has been sent and NACKed.
 */
#define TWI_MT_SLA_NACK 0x20

/**
 * \brief Data has been sent and ACKed.
 */
#define TWI_MT_DATA_ACK 0x28

/**
 * \brief Data has been sent and NACKed.
 */
#define TWI_MT_DATA_NACK 0x30

/**
 * \brief Bus arbitration has been lost.
 * \warning Transmission has to be restarted or stopped!
 * 
 * The bus is lost in sending the SLA+W or by sending a data byte.
 */
#define TWI_MASTER_ARB_LOST 0x38

//  ********************************
//  * Master receiver              *
//  ********************************

/**
 * \brief Slave address has been sent and ACKed.
 */
#define TWI_MR_SLA_ACK 0x40

/**
 * \brief Slave address has been sent and NACKed.
 */
#define TWI_MR_SLA_NACK 0x48

/**
 * \brief Data has been received and ACK is returned.
 */
#define TWI_MR_DATA_ACK 0x50

/**
 * \brief Data has been received and NACK is returned.
 */
#define TWI_MR_DATA_NACK 0x58

/**
 * \def TWGO
 * \brief Enable the TWI interface + interrupt.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN
 * * TWIEN
 * * TWEA
 */
#define TW_ENABLE (BIT(0) | BIT(2) | BIT(6) | BIT(7))

/**
 * \def TWGO
 * \brief Enable the TWI interface and sent a start condition.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN
 * * TWIEN
 * * TWSTO
 * * TWEA
 */
#define TW_STOP (TW_ENABLE | BIT(4))

/**
 * \def TWGO
 * \brief Enable the TWI interface and sent a start condition.
 * 
 * The TWI interface will be enabled by setting the following bits: \n
 * * TWEN
 * * TWIEN
 * * TWSTA
 * * TWEA
 */
#define TWGO (TW_ENABLE | BIT(5))

#define TW_RELEASE (TW_ENABLE & (~BIT(0)))

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
extern void BermudaTwi0Init(TWIBUS *bus);
extern int BermudaTwiMasterTransfer(TWIBUS *twi, const void *tx, unsigned int txlen,  
							  void *rx, unsigned int rxlen, unsigned char sla,
							  uint32_t frq, unsigned int tmo);
extern unsigned char BermudaTwiCalcTWBR(uint32_t freq, unsigned char pres);
extern unsigned char BermudaTwiCalcPres(uint32_t pres);

PRIVATE WEAK void BermudaTwInit(TWIBUS *twi, const void *tx, unsigned int txlen, 
								void *rx, unsigned int rxlen, unsigned char sla, 
								uint32_t frq);
PRIVATE WEAK void BermudaTwiArbitrationLost(TWIBUS *twi);
PRIVATE WEAK int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);
PRIVATE WEAK void BermudaTwiISR(TWIBUS *twi);
__DECL_END

#endif
