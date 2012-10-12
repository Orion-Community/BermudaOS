/*
 *  BermudaOS - ATmega I2C private header
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

/**
 * \file include/dev/i2c/busses/atmega_priv.h ATmega private header.
 * \brief Header file for the I2C bus of the megaAVR architecture.
 */

#ifndef __I2C_ATMEGA_PRIV_H_
#define __I2C_ATMEGA_PRIV_H_

#include <stdlib.h>
#include <stdio.h>

#include <lib/binary.h>

#include <arch/io.h>

/**
 * \brief The maximum I2C busses an ATmega MCU can have.
 */
#define ATMEGA_BUSSES 1

/* I2C device I/O control features */
#define I2C_START		BIT(0)
#define I2C_STOP		BIT(1)

#define I2C_ACK			BIT(2)
#define I2C_LISTEN		I2C_ACK
#define I2C_NACK		BIT(3)
#define I2C_IDLE		I2C_NACK
#define I2C_RELEASE		BIT(4)

#define I2C_BLOCK		BIT(5)
#define I2C_RESET		BIT(6)


/* End of I/O control features */

#define I2C_SLA_READ_BIT BIT(0)
#define I2C_SLA_WRITE_MASK (~BIT(0))

/**
 * \brief Name of the ATmega I2C I/O file.
 */
#define I2C_FNAME "I2C_ATMEGA_C0"

//  ********************************
//  * Master transmitter           *
//  ********************************

/**
 * \brief Start has been sent.
 */
#define I2C_MASTER_START 0x8

/**
 * \brief Repeated start has been sent.
 */
#define I2C_MASTER_REP_START 0x10

/**
 * \brief Slave address has been sent and ACKed.
 */
#define I2C_MT_SLA_ACK 0x18

/**
 * \brief Slave address has been sent and NACKed.
 */
#define I2C_MT_SLA_NACK 0x20

/**
 * \brief Data has been sent and ACKed.
 */
#define I2C_MT_DATA_ACK 0x28

/**
 * \brief Data has been sent and NACKed.
 */
#define I2C_MT_DATA_NACK 0x30

/**
 * \brief Bus arbitration has been lost.
 * \warning Transmission has to be restarted or stopped!
 * 
 * The bus is lost in sending the SLA+W or by sending a data byte.
 */
#define I2C_MASTER_ARB_LOST 0x38

//  ********************************
//  * Master receiver              *
//  ********************************

/**
 * \brief Slave address has been sent and ACKed.
 */
#define I2C_MR_SLA_ACK 0x40

/**
 * \brief Slave address has been sent and NACKed.
 */
#define I2C_MR_SLA_NACK 0x48

/**
 * \brief Data has been received and ACK is returned.
 */
#define I2C_MR_DATA_ACK 0x50

/**
 * \brief Data has been received and NACK is returned.
 */
#define I2C_MR_DATA_NACK 0x58

//  ********************************
//  * Slave receiver               *
//  ********************************

#define I2C_SR_SLAW_ACK 0x60 //!< Own slave address has been received and ACKed.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; own SLA+W has been 
 * received; ACK has been returned.
 */
#define I2C_SR_SLAW_ARB_LOST 0x68
#define I2C_SR_GC_ACK 0x70 //!< General call received, ACK returned.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; General call address has 
 * been received; ACK has been returned.
 */
#define I2C_SR_GC_ARB_LOST 0x78

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and an ACK is
 * returned.
 */
#define I2C_SR_SLAW_DATA_ACK 0x80

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and a NACK is
 * returned.
 */
#define I2C_SR_SLAW_DATA_NACK 0x88

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and an ACK is
 * returned.
 */
#define I2C_SR_GC_DATA_ACK 0x90

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and a NACK is
 * returned.
 */
#define I2C_SR_GC_DATA_NACK 0x98
#define I2C_SR_STOP 0xA0 //!< Stop or repeated start condition received.

//  ********************************
//  * Slave Transmitter            *
//  ********************************

#define I2C_ST_SLAR_ACK 0xA8 //!< Own SLA+R received, ack returned.
#define I2C_ST_ARB_LOST 0xB0 //!< Arbitration lost as master, own SLA+R received.
#define I2C_ST_DATA_ACK 0xB8 //!< Data sent successfuly, ACK received.
#define I2C_ST_DATA_NACK 0xC0 //!< Data has been sent, NACK received.
#define I2C_ST_LAST_DATA_ACK 0xC8 //!< The last data byte is transmitted, ACK received.

#define I2C_BUS_ERROR 0x0 //!< Generic bus error.

/**
 * \def TWBR
 * \brief TW bit rate register.
 * \see BermudaTwiCalcTWBR
 * 
 * Set the bit rate. Usually calculated using BermudaTwiCalcTWBR.
 */
#define TWBR  MEM_IO8(0xB8)

/**
 * \def TWCR
 * \brief TW control register.
 * \note Most operations on TWCR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Controls the Two Wire Interface.
 */
#define TWCR  MEM_IO8(0xBC)

/**
 * \def TWSR
 * \brief TW status register.
 * \note Most operations on TWSR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 */
#define TWSR  MEM_IO8(0xB9)

/**
 * \def TWDR
 * \brief TW data register.
 * \note All operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWDR  MEM_IO8(0xBB)

/**
 * \def TWAR
 * \brief TW SLA register.
 * \note Most operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWAR  MEM_IO8(0xBA)

/**
 * \def TWAMR
 * \brief TW SLA mask register
 * 
 * Masks bits in the TWAR.
 */
#define TWAMR MEM_IO8(0xBD)

/*
 * Prescalers
 */

// prescaler defines
/**
 * \def I2C_PRES_1
 * \brief Prescaler of 1. 
 * 
 * This value has no effect on the prescaler value.
 */
#define I2C_PRES_1 B0

/**
 * \def I2C_PRES_4
 * \brief Prescaler value 4.
 */
#define I2C_PRES_4 B1

/**
 * \def I2C_PRES_16
 * \brief Prescaler value 16
 */
#define I2C_PRES_16 B10

/**
 * \def I2C_PRES_64
 * \brief Prescaler value 64
 */
#define I2C_PRES_64 B11

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
#define I2C_FRQ(x, n) \
	(F_CPU/(16+(2*x*n)))
	
#define I2C_CALC_TWBR(__freq, __pres) \
(F_CPU - (16*__freq)) / (2*__pres*__freq)
#endif

/* control reg */
#define TWINT	7
#define TWEA	6
#define TWSTA	5
#define TWSTO	4
#define TWWC	3
#define TWEN	2
#define TWIE	0

/* status reg */
#define TWPS1	1
#define TWPS0	0

/* addr reg */
#define TWGCE 	0


/* ---------------------- */

/**
 * \brief The I2C megaAVR private structure.
 * 
 * This structure contains private data, such as I/O registers, which should be
 * hidden from higher layers. It contains an I/O file for communcation purposes.
 */
struct atmega_i2c_priv
{
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +----------------------------------------------------------+\n
	 * | TWINT | TWEA | TWSTA | TWSTO | TWWC |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;| 
	 * TWEN | TWIE |\n
	 * +----------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twcr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +---------------------------------------------------------+\n
	 * | TWS7 | TWS6 | TWS5 | TWS4 | TWS3 |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	 * &nbsp;| TWPS1 | TWPS0 |\n
	 * +---------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twsr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------+\n
	 * | TWD7 | TWD6 | TWD5 | TWD4 | TWD3 | TWD2 | TWD1 | TWD0 |\n
	 * +-------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twdr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------+\n
	 * | TWB7 | TWB6 | TWB5 | TWB4 | TWB3 | TWB2 | TWB1 | TWB0 |\n
	 * +-------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twbr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +--------------------------------------------------------+\n
	 * | TWA6 | TWA5 | TWA4 | TWA3 | TWA2 | TWA1 | TWA0 | TWGCE |\n
	 * +--------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twar;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------------+\n
	 * | TWAM6 | TWAM5 | TWAM4 | TWAM3 | TWAM2 | TWAM1 | TWAM0 |&nbsp;&nbsp;
	 * &nbsp;&nbsp;&nbsp;&nbsp;|\n
	 * +-------------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twamr;
	
	/**
	 * \brief The interrupt service routine which should be called.
	 * \param adap The bus adapter.
	 * 
	 * This function should <b>ONLY</b> be called by hardware.
	 */
	void (*isr)(struct i2c_adapter *adap);
} __attribute__((packed));

__DECL
static inline int atmega_i2c_reg_write(reg8_t reg, uint8_t *data)
{
	if(reg) {
		outb(reg, *data);
		return 0;
	} else {
		return -1;
	}
}

static inline int atmega_i2c_reg_read(reg8_t reg, uint8_t *data)
{
	if(reg) {
		inb(reg, data);
		return 0;
	} else {
		return -1;
	}

}

static inline uint8_t atmega_i2c_get_status(struct atmega_i2c_priv *priv)
{
	unsigned char status = 0;
	atmega_i2c_reg_read(priv->twsr, &status);
	return (status & B11111000);
}

static inline size_t atmega_i2c_get_index(FILE *stream)
{
	return stream->index;
}

static inline void atmega_i2c_inc_index(FILE *stream)
{
	stream->index++;
}

static inline void atmega_i2c_reset_index(FILE *stream)
{
	stream->index = 0;
}

static inline void atmega_i2c_set_bitrate(struct atmega_i2c_priv *priv,
										  uint8_t twbr, uint8_t twps)
{
	twps &= B11;
	
	atmega_i2c_reg_write(priv->twbr, &twbr);
	atmega_i2c_reg_write(priv->twsr, &twps);
}

ISR_DEF(atmega_i2c_isr_handle, adapter, struct i2c_adapter *);

__DECL_END

#endif /* __I2C_ATMEGA_PRIV_H_ */