/*
 *  BermudaOS - I2C device driver
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

//! \file src/dev/i2c/busses/atmega-i2c.c ATmega I2C bus driver.

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/i2c/busses/atmega.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <fs/vfile.h>

#include <arch/io.h>
#include <arch/irq.h>

#include "atmega_priv.h"

/* static functions */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data);
static int atmega_i2c_slave_respond(FILE *stream);
static int atmega_i2c_slave_listen(struct i2c_adapter *adpater, FILE *stream);

static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres);
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq);


static int atmega_i2c_put_char(int c, FILE *stream);
static int atmega_i2c_get_char(FILE *stream);
static int atmega_i2c_init_transfer(FILE *stream);

/**
 * \brief File I/O structure for bus 0 on port C.
 */
static FDEV_SETUP_STREAM(i2c_c0_io, NULL, NULL, &atmega_i2c_put_char, 
						 &atmega_i2c_get_char, &atmega_i2c_init_transfer, 
						 I2C_FNAME, 0 /* flags */, NULL /* data */);

/**
 * \brief I2C control structure for the bus 0 on port C.
 */
static struct atmega_i2c_priv i2c_c0 = {
	.twcr = &TWCR,
	.twdr = &TWDR,
	.twsr = &TWSR,
	.twar = &TWAR,
	.twbr = &TWBR,
	.twamr = &TWAMR,
};

#ifdef __THREADS__
/**
 * \brief I2C bus 0 mutex.
 */
static volatile void *bus_c0_mutex = SIGNALED;
/**
 * \brief I2C bus 0 master queue.
 */
static volatile void *bus_c0_master_queue = SIGNALED;
/**
 * \brief I2C bus 0 slave queue.
 */
static volatile void *bus_c0_slave_queue = SIGNALED;
#endif

/**
 * \brief Array of all available I2C busses
 */
static struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

/**
 * \brief I2C bus 0 message array.
 */
static struct i2c_message *i2c_c0_msgs[I2C_MSG_NUM] = { NULL, NULL, NULL, NULL, };

/**
 * \brief Intialize an I2C bus.
 * \param sla Slave address of this bus.
 * \param adapter Bus adapter.
 * 
 * This function will initialize bus 0 on port c.
 */
PUBLIC void atmega_i2c_c0_hw_init(uint8_t sla, struct i2c_adapter *adapter)
{
	int rc = i2c_init_adapter(adapter, i2c_c0_io.name);
	if(rc < 0) {
		return;
	}

	atmega_i2c_busses[0] = adapter;
#ifdef __THREADS__
	adapter->dev->mutex = &bus_c0_mutex;
#endif
	adapter->dev->ctrl = &atmega_i2c_ioctl;
	adapter->dev->io = &i2c_c0_io;
	
	adapter->dev->io->buff = (void*)i2c_c0_msgs;
#ifdef __THREADS__
	adapter->master_queue = &bus_c0_master_queue;
	adapter->slave_queue = &bus_c0_slave_queue;
#else
	adapter->mutex = 0;
	adapter->master_queue = 1;
	adapter->slave_queue = 1;
#endif
	adapter->data = (void*)&i2c_c0;
	adapter->slave_respond = &atmega_i2c_slave_respond;

	vfs_add(&i2c_c0_io);
	open(i2c_c0_io.name, _FDEV_SETUP_RW);
	
	adapter->dev->ctrl(adapter->dev, I2C_IDLE, NULL);
	atmega_i2c_reg_write(i2c_c0.twar, &sla);
}

/**
 * \brief Initialise a I2C client structure.
 * \param client Client to initialize.
 * \param ifac BUs to initialize the client for.
 */
PUBLIC void atmega_i2c_init_client(struct i2c_client *client, uint8_t ifac)
{
	client->adapter = atmega_i2c_busses[ifac];
}

/**
 * \brief Calculate the value of the TWBR register.
 * \param freq Wanted frequency.
 * \param pres Used prescaler.
 * \note The <b>pres</b> parameter can have one of the following values: \n
 *       * I2C_PRES_1 \n
 *       * I2C_PRES_4 \n
 *       * I2C_PRES_16 \n
 *       * I2C_PRES_64
 * \see I2C_PRES_1
 * \see I2C_PRES_4 
 * \see I2C_PRES_16 
 * \see I2C_PRES_64
 * 
 * The needed value of the TWBR register will be calculated using the given
 * (and used) prescaler value.
 */
static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres)
{
	char prescaler;
	uint32_t twbr;
	
	switch(pres) {
		case I2C_PRES_1:
			prescaler = 1;
			break;
		case I2C_PRES_4:
			prescaler = 4;
			break;
		case I2C_PRES_16:
			prescaler = 16;
			break;
		case I2C_PRES_64:
			prescaler = 64;
			break;
		default:
			prescaler = -1;
			break;
	}
	
	if(prescaler == -1) {
		return 0xFF;
	}
	
	twbr = I2C_CALC_TWBR(freq, prescaler);
	
	return twbr & 0xFF;
}

/**
 * \brief Calculates the I2C prescaler value.
 * \param frq The desired frequency.
 * \return The prescaler value in hardware format: \n
 *         * B0 for a prescaler of 1; \n
 *         * B1 for a prescaler of 4; \n
 *         * B10 for a prescaler of 16; \n
 *         * B11 for a prescaler of 64
 *
 * Calculates the prescaler value based on the given frequency.
 */
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq)
{
	unsigned char ret = 0;
	
	if(frq > I2C_FRQ(255,1)) {
		ret = B0;
	}
	else if(frq > I2C_FRQ(255,4) && frq < I2C_FRQ(1,4)) {
		ret = B11;
	}
	else if(frq > I2C_FRQ(255,16) && frq < I2C_FRQ(1,16)) {
		ret = B10;
	}
	else if(frq > I2C_FRQ(255,64) && frq < I2C_FRQ(1,64)) {
		ret = B1;
	}

	return ret;
}

/**
 * \brief Start a master transfer.
 * \param stream The device I/O file.
 * 
 * This function is called by the user using the <b>flush(fd : int)</b> funtion and will start the
 * actial transfer.
 */
static int atmega_i2c_init_transfer(FILE *stream)
{
	auto void config_bitrate();
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->data;
	uint8_t pres, twbr, reg = 0;
	int rc = -1;
	
	if((stream->flags & I2C_MASTER) != 0) {
		if(!adap->busy) {
			reg = atmega_i2c_get_status(priv);
			if(reg == I2C_NOINFO) {
				atmega_i2c_reg_read(priv->twcr, &reg);
				config_bitrate();
				if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
					adap->dev->ctrl(adap->dev, I2C_START | I2C_ACK, NULL);
				} else {
					adap->dev->ctrl(adap->dev, I2C_START | I2C_NACK, NULL);
				}
			}
		}

#ifdef __THREADS__
		rc = BermudaEventWaitNext( (volatile THREAD**)adap->master_queue, I2C_TMO);
#else
		adap->master_queue = 1;
		BermudaIoWait(&(adap->master_queue));
		rc = 0;
#endif
		
	} else if((stream->flags & I2C_SLAVE) != 0) {
		rc = atmega_i2c_slave_listen(adap, stream);
	}
	
	return rc;
	
	auto void config_bitrate()
	{
		pres = atmega_i2c_calc_prescaler(client->freq);
		twbr = atmega_i2c_calc_twbr(client->freq, pres);
		atmega_i2c_set_bitrate(priv, twbr, pres);
	}
}

static int atmega_i2c_slave_listen(struct i2c_adapter *adapter, FILE *stream)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	struct atmega_i2c_priv *priv = adapter->data;
	uint8_t status;
	int rc = -1;
	
	if(!adapter->busy) {
		status = atmega_i2c_get_status(priv);
		if(status == I2C_NOINFO) {
			if(msgs[I2C_MASTER_TRANSMIT_MSG] || msgs[I2C_MASTER_RECEIVE_MSG]) {
				adapter->dev->ctrl(adapter->dev, I2C_START | I2C_ACK, NULL);
			} else {
				adapter->dev->ctrl(adapter->dev, I2C_LISTEN, NULL);
			}
		}
	}
	
#ifdef __THREADS__
	rc = BermudaEventWaitNext( (volatile THREAD**)adapter->slave_queue, 500);
#else
	adapter->slave_queue = 1;
	BermudaIoWait(&(adapter->slave_queue));
	rc = 0;
#endif
	return rc;
}

/**
 * \brief Respond to the master after a slave receive transfer.
 * \param stream File stream.
 */
static int atmega_i2c_slave_respond(FILE *stream)
{
	int rc;
	auto void no_tx();
	struct i2c_message **msgs = (struct i2c_message**)stream->buff;
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adapter = client->adapter;
	
	if(msgs[I2C_SLAVE_TRANSMIT_MSG]) {
		if(msgs[I2C_SLAVE_TRANSMIT_MSG]->length && msgs[I2C_SLAVE_TRANSMIT_MSG]->buff) {
			adapter->dev->ctrl(adapter->dev, I2C_LISTEN, NULL);
#ifdef __THREADS__
			if((rc = BermudaEventWaitNext( (volatile THREAD**)adapter->slave_queue, I2C_TMO)) == -1) {
				return rc;
			}
#else
			adapter->slave_queue = 1;
			BermudaIoWait(&(adapter->slave_queue));
			rc = 0;
#endif
		} else {
			no_tx();
			i2c_cleanup_msg(stream, I2C_SLAVE_TRANSMIT_MSG);
		}
	} else {
		no_tx();
	}
	
	return rc;
	
	auto void no_tx()
	{
		if(msgs[I2C_MASTER_TRANSMIT_MSG] || msgs[I2C_MASTER_RECEIVE_MSG]) {
			stream->flags &= ~I2C_SLAVE;
			stream->flags |= I2C_MASTER;
			adapter->dev->ctrl(adapter->dev, I2C_START | I2C_ACK, NULL);
			rc = 0;
		} else {
			adapter->dev->ctrl(adapter->dev, I2C_RELEASE, NULL);
			rc = 0;
		}
	}
}

/**
 * \brief ATmega I2C bus controller.
 * \param dev Bus device.
 * \param cfg Config options.
 * \param data Configuration data.
 */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data)
{
	struct i2c_adapter *adap = dev->ioctl;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t reg = 0;
	
	atmega_i2c_reg_read(priv->twcr, &reg);
	
	switch((uint16_t)cfg) {
		/* transaction control */
		case I2C_START | I2C_ACK:
			reg |= BIT(TWSTA) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_START | I2C_NACK:
			reg = BIT(TWSTA) | BIT(TWINT) | BIT(TWIE) | BIT(TWEN) | (reg & ~BIT(TWEA));
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_STOP | I2C_ACK:
			reg |= BIT(TWSTO) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_STOP | I2C_NACK:
			reg = (reg & ~BIT(TWEA)) | BIT(TWSTO) | BIT(TWINT) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
		
		/* bus control */
		case I2C_RELEASE:
			reg |= BIT(TWEN) | BIT(TWIE) | BIT(TWINT);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_ACK:
			reg |= BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_NACK:
			reg = (reg & ~BIT(TWEA)) | BIT(TWINT) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;

		case I2C_BLOCK:
			reg &= ~(BIT(TWINT) | BIT(TWIE));
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_RESET:
			reg |= BIT(TWSTO);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		default:
			break;
	}
	return;
}

/**
 * \brief File I/O put function.
 * \param c Byte to write.
 * \param stream File to write to.
 * 
 * Write one byte to the data register of the addressed I2C bus.
 */
static int atmega_i2c_put_char(int c, FILE *stream)
{
	struct i2c_adapter *adap = ((struct i2c_client*)stream->data)->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t data = (uint8_t)c&0xFF;
	atmega_i2c_reg_write(priv->twdr, &data);
	
	return c;
}

/**
 * \brief File I/O get function.
 * \param stream File to read from.
 * 
 * Read one byte from the data register of the addressed I2C bus.
 */
static int atmega_i2c_get_char(FILE *stream)
{
	struct i2c_adapter *adap = ((struct i2c_client*)stream->data)->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t data = 0;
	
	atmega_i2c_reg_read(priv->twdr, &data);
	
	return (int)(data&0xFF);
}

/**
 * \brief The handler of ATmega I2C busses.
 * \param adapter I2C adapter which belongs to the calling bus.
 * \note The name of this function is <i><b>atmega_i2c_isr_handle</b></i>.
 * \warning This function should only be called by hardware.
 */
ISR(atmega_i2c_isr_handle, adapter, struct i2c_adapter *)
{
	struct atmega_i2c_priv *priv = (struct atmega_i2c_priv*)adapter->data;
	struct device *dev = adapter->dev;
	FILE *stream = dev->io;
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	uint8_t status = atmega_i2c_get_status(priv);
	const int dummy = 0;
	int i;
	
	if(!stream) {
		return;
	}
	
	/* switch for the status to handle the ISR transaction. */
	switch(status) {
		/*
		 * I2C_MASTER_START: Start sent with success.
		 * I2C_MASTER_REP_START Repeated start has been sent with success.
		 */
		case I2C_MASTER_START:
		case I2C_MASTER_REP_START:
			atmega_i2c_reset_index(stream);
			adapter->flags = I2C_MASTER_ENABLE;
			adapter->busy = true;
			stream->flags &= ~I2C_SLAVE;
			stream->flags |= I2C_MASTER;
			
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if(msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					adapter->flags &= ~I2C_TRANSMITTER;
					adapter->flags |= I2C_RECEIVER;
					msgs[I2C_MASTER_RECEIVE_MSG]->addr |= I2C_SLA_READ_BIT;
					status = msgs[I2C_MASTER_RECEIVE_MSG]->addr; /* use status as a temp variable */
				}
			}
			/* 
			 * master transmitter has a higher priority than master receiver, 
			 * therefore this if must be last 
			 */
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				if(msgs[I2C_MASTER_TRANSMIT_MSG]->length) {
					adapter->flags &= ~I2C_RECEIVER;
					adapter->flags |= I2C_TRANSMITTER;
					msgs[I2C_MASTER_TRANSMIT_MSG]->addr &= I2C_SLA_WRITE_MASK;
					status = msgs[I2C_MASTER_TRANSMIT_MSG]->addr; /* use status as  temp variable */
					
				}
			}

			fputc((int)status, stream); /* sent the slave address */
			atmega_i2c_reg_read(priv->twcr, &status);
			/* disable the start bit */
			status = (status & ~BIT(TWSTA)) | BIT(TWEN) | BIT(TWIE) | BIT(TWINT) | BIT(TWEA);
			atmega_i2c_reg_write(priv->twcr, &status);
			break;
		/*
		 * I2C_MT_SLA_ACK: Slave address has been transmitted and ACK is returned.
		 * I2C_MT_DATA_ACK: Data byte has been transmitted and ACK is returned.
		 */
		case I2C_MT_SLA_ACK:
		case I2C_MT_DATA_ACK:
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				if(atmega_i2c_get_index(stream) < msgs[I2C_MASTER_TRANSMIT_MSG]->length) {
					fputc((int)msgs[I2C_MASTER_TRANSMIT_MSG]->buff[atmega_i2c_get_index(stream)], 
						stream);
					atmega_i2c_inc_index(stream);
					dev->ctrl(dev, I2C_ACK, NULL);
					break;
				} else if(msgs[I2C_MASTER_RECEIVE_MSG]) { 
					if(msgs[I2C_MASTER_RECEIVE_MSG]->length) {
						adapter->flags &= ~I2C_TRANSMITTER;
						adapter->flags |= I2C_RECEIVER;
						msgs[I2C_MASTER_TRANSMIT_MSG]->length = 0;
						i2c_cleanup_msg(stream, I2C_MASTER_TRANSMIT_MSG);
						
						/* sent the repeated start */
						atmega_i2c_reg_read(priv->twcr, &status);
						dev->ctrl(dev, I2C_START | ((status & BIT(TWEA)) >> 4), NULL);
						break;
					}
				}
			}
			/* else: no space to write and no master receive operation is pending */
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
#endif
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				msgs[I2C_MASTER_TRANSMIT_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_MASTER_TRANSMIT_MSG);
			}
			adapter->busy = false;
			
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				if(msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
					adapter->flags = I2C_SLAVE_ENABLE;
					adapter->flags |= I2C_RECEIVER;
					dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
					break;
				}
			} 
			/* no slave operation pending */
			adapter->flags = 0;
			dev->ctrl(dev, I2C_STOP | I2C_IDLE, NULL);
			break;
			
		/*
		 * I2C_MASTER_ARB_LOST: Lost arbitration as master. Sent another start when the interface
		 *                      is available again.
		 */
		case I2C_MASTER_ARB_LOST:
			atmega_i2c_reg_read(priv->twcr, &status);
			dev->ctrl(dev, I2C_START | ((status & BIT(TWEA)) >> 4), NULL);
			adapter->busy = 0;
			break;
		
		/*
		 * I2C_MT_SLA_NACK: Slave address has been transmitted, NACK is returned by the slave.
		 * I2C_MT_DATA_NACK: Data byte has been transmitted as master, NACK is returned.
		 */
		case I2C_MT_SLA_NACK:
		case I2C_MT_DATA_NACK:
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				msgs[I2C_MASTER_TRANSMIT_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_MASTER_TRANSMIT_MSG);
			}
			/* roll through */
		/*
		 * I2C_MR_SLA_NACK: Slave address has been transmitted, NACK is returned by the slave.
		 */
		case I2C_MR_SLA_NACK:
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				msgs[I2C_MASTER_RECEIVE_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_MASTER_RECEIVE_MSG);
			}
			adapter->busy = false;
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
#endif
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				if(msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
					adapter->flags = I2C_SLAVE_ENABLE;
					adapter->flags |= I2C_RECEIVER;
					dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
					break;
				} 
			}
			
			adapter->flags = 0;
			dev->ctrl(dev, I2C_STOP | I2C_IDLE, NULL);

			adapter->flags |= I2C_ERROR;
			break;
			
		/*
		 * I2C_MR_DATA_ACK: Data received as master, ACK returned.
		 */
		case I2C_MR_DATA_ACK:
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if(atmega_i2c_get_index(stream) < msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					msgs[I2C_MASTER_RECEIVE_MSG]->buff[atmega_i2c_get_index(stream)] = (uint8_t)fgetc(stream);
					atmega_i2c_inc_index(stream);
				}
			}
			/* fall through to sla ack */
		/* 
		 * I2C_MR_SLA_ACK: Slave address has been ACKed by the slave.
		 */
		case I2C_MR_SLA_ACK:
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if((atmega_i2c_get_index(stream) + 1) < msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					/*
					* ACK if there is more than 1 byte to transfer.
					*/
					dev->ctrl(dev, I2C_ACK, NULL);
				} else {
					dev->ctrl(dev, I2C_NACK, NULL);
				}
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
			}
			break;
			
		/*
		 * I2C_MR_DATA_NACK: Data received as a master, NACK returned.
		 */
		case I2C_MR_DATA_NACK:
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if(atmega_i2c_get_index(stream) < msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					msgs[I2C_MASTER_RECEIVE_MSG]->buff[atmega_i2c_get_index(stream)] = (uint8_t)fgetc(stream);
				}
				
				msgs[I2C_MASTER_RECEIVE_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_MASTER_RECEIVE_MSG);
			}
			adapter->busy = false;
			
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
#endif
			
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				if(msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
					dev->ctrl(dev, I2C_LISTEN | I2C_STOP, NULL);
					adapter->flags = I2C_SLAVE_ENABLE;
					adapter->flags |= I2C_RECEIVER;
					break;
				} 
			}
			
			/* no slave operation pending, enter sent stop and enter idle mode */
			dev->ctrl(dev, I2C_IDLE | I2C_STOP, NULL);
			adapter->flags = 0;
			break;
			
		/*
		 * I2C_SR_SLAW_ACK: Device slave address received, ACK returned.
		 * I2C_SR_GC_ACK: General call address received.
		 * I2C_SR_GC_ARB_LOST: Arbitration lost as a master, received general call address.
		 * I2C_SR_SLAW_ARB_LOST: Arbitration lost as a master, received own slave address.
		 */
		case I2C_SR_SLAW_ACK:
		case I2C_SR_GC_ACK:
		case I2C_SR_GC_ARB_LOST:
		case I2C_SR_SLAW_ARB_LOST:
			atmega_i2c_reset_index(stream);
			adapter->flags = I2C_RECEIVER | I2C_SLAVE_ENABLE;
			adapter->busy = true;
			
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				if(msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
					dev->ctrl(dev, I2C_ACK, NULL);
				} else {
					dev->ctrl(dev, I2C_NACK, NULL);
					adapter->flags |= I2C_ERROR;
				}
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
				adapter->flags |= I2C_ERROR;
			}
			break;
			
		/*
		 * I2C_SR_SLAW_DATA_ACK: Received a data byte while being addressed with its own 
		 *                       slave address.
		 * I2C_SR_GC_DATA_ACK: Received a data byte while being addressed with the general
		 *                     call address.
		 */
		case I2C_SR_SLAW_DATA_ACK:
		case I2C_SR_GC_DATA_ACK:
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				if(atmega_i2c_get_index(stream) < msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
					msgs[I2C_SLAVE_RECEIVE_MSG]->buff[atmega_i2c_get_index(stream)] = (uint8_t)fgetc(stream);
					
					if(atmega_i2c_get_index(stream) + 1 < msgs[I2C_SLAVE_RECEIVE_MSG]->length) {
						dev->ctrl(dev, I2C_ACK, NULL);
					} else {
						dev->ctrl(dev, I2C_NACK, NULL);
					}
					atmega_i2c_inc_index(stream);
					break;
				}
			}
			
			/*
			 * If there are no more data bytes to receive, we are at the end of our transfer. In
			 * this case we will fall through to the NACKs to end the transfer and wake up the
			 * user application.
			 */
		/*
		 * I2C_SR_SLAW_DATA_NACK: Received a data byte, NACK is returned. The device is being
		 *                        addressed with its own slave address.
		 * I2C_SR_GC_DATA_NACK: Received a data byte, NACK is returned. The device is being
		 *                      addressed with is own slave address.
		 */
		case I2C_SR_SLAW_DATA_NACK:
		case I2C_SR_GC_DATA_NACK:
			if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
				msgs[I2C_SLAVE_RECEIVE_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_SLAVE_RECEIVE_MSG);
			}
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				if(msgs[I2C_MASTER_TRANSMIT_MSG]->length) {
					adapter->flags = 0;
					dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
					break;
				}
			} 
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if(msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					adapter->flags = 0;
					dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
					break;
				}
			} 
			/* No master operation is pending. In this case we will keep NACKing to the master */
			dev->ctrl(dev, I2C_NACK, NULL);
			break;
			
		/*
		 * I2C_SR_STOP: Received STOP bit from the master. End of transfer.
		 */
		case I2C_SR_STOP:
			/*
			 * application gave up waiting
			 */
			if(*(adapter->slave_queue) == NULL || *(adapter->slave_queue) == SIGNALED) {
				if(msgs[I2C_MASTER_TRANSMIT_MSG] || msgs[I2C_MASTER_RECEIVE_MSG]) {
					dev->ctrl(dev, I2C_START | I2C_NACK, NULL);
				} else {
					dev->ctrl(dev, I2C_IDLE, NULL);
				}
				adapter->busy = false;
			} else {
				if(msgs[I2C_SLAVE_RECEIVE_MSG]) {
					dev->ctrl(dev, I2C_BLOCK, NULL);
					msgs[I2C_SLAVE_RECEIVE_MSG]->length = 0;
					i2c_cleanup_msg(stream, I2C_SLAVE_RECEIVE_MSG);
				}
				adapter->flags &= ~I2C_ERROR;
				adapter->flags |= I2C_CALL_BACK;
				dev->ctrl(dev, I2C_BLOCK, NULL);
#ifdef __THREADS__
				BermudaEventSignalFromISR( (volatile THREAD**)adapter->slave_queue);
#else
				BermudaIoSignal(&(adapter->slave_queue));
#endif
			}
			break;
			
		/* 
		 * I2C_ST_ARB_LOST: Lost abitration as a master, received SLA+R.
		 * I2C_ST_SLAR_ACK: Received SLA+R from master and returned ACK.
		 */
		case I2C_ST_ARB_LOST:
		case I2C_ST_SLAR_ACK:
			atmega_i2c_reset_index(stream);
			adapter->busy = true;
			adapter->flags = I2C_SLAVE_ENABLE | I2C_TRANSMITTER;
			
			/* roll through for first byte */
		/*
		 * I2C_ST_DATA_ACK: Transmitted a data byte and ACK has been returned by the master.
		 */
		case I2C_ST_DATA_ACK:
			if(msgs[I2C_SLAVE_TRANSMIT_MSG]) {
				if(atmega_i2c_get_index(stream) < msgs[I2C_SLAVE_TRANSMIT_MSG]->length) {
					fputc((int)msgs[I2C_SLAVE_TRANSMIT_MSG]->buff[atmega_i2c_get_index(stream)], 
						  stream);
					if(atmega_i2c_get_index(stream) + 1 < msgs[I2C_SLAVE_TRANSMIT_MSG]->length) {
						dev->ctrl(dev, I2C_ACK, NULL);
					} else {
						dev->ctrl(dev, I2C_NACK, NULL);
					}
					atmega_i2c_inc_index(stream);
				} else {
					fputc(dummy, stream);
					dev->ctrl(dev, I2C_NACK, NULL);
				}
			} else {
				fputc(dummy, stream);
				dev->ctrl(dev, I2C_NACK, NULL);
				break;
			}
			break;
		
		/*
		 * I2C_ST_DATA_NACK: Transmitted data byte, NACK is returned.
		 * I2C_ST_LAST_DATA_ACK: Received an ACK after transmitting the last data byte. After this
		 *                       byte a STOP is issued by the master.
		 */
		case I2C_ST_DATA_NACK:
		case I2C_ST_LAST_DATA_ACK:
			adapter->flags &= ~I2C_ERROR;
			if(msgs[I2C_SLAVE_TRANSMIT_MSG]) {
				msgs[I2C_SLAVE_TRANSMIT_MSG]->length = 0;
				i2c_cleanup_msg(stream, I2C_SLAVE_TRANSMIT_MSG);
			}
			adapter->busy = false;
			
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->slave_queue);
#else
			BermudaIoSignal(&(adapter->slave_queue));
#endif
			
			/* check for pending master operations */
			if(msgs[I2C_MASTER_TRANSMIT_MSG]) {
				if(msgs[I2C_MASTER_TRANSMIT_MSG]->length) {
					adapter->flags = 0;
					dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
					break;
				}
			}
			if(msgs[I2C_MASTER_RECEIVE_MSG]) {
				if(msgs[I2C_MASTER_RECEIVE_MSG]->length) {
					adapter->flags = 0;
					dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
					break;
				}
			} 
			dev->ctrl(dev, I2C_IDLE, NULL);
			break;
		
		/*
		 * I2C_BUS_ERROR: A fatal error has occured.
		 */
		case I2C_BUS_ERROR:
		default:
			/*
			 * A fatal error has occured, so all (waiting) transfers will have to be ceased.
			 * Therefore, all messages will be invallidated and marked for clean up.
			 */
			for(i = 0; i < I2C_MSG_NUM; i++) {
				msgs[i]->buff = NULL;
				msgs[i]->length = 0;
				i2c_cleanup_msg(stream, i);
			}
			
			atmega_i2c_reset_index(stream);
			dev->ctrl(dev, I2C_RESET, NULL); // reset the interface
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->slave_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
			BermudaIoSignal(&(adapter->slave_queue));
#endif
			break;
	}
	return;
}

SIGNAL(TWI_STC_vect)
{
	atmega_i2c_isr_handle(atmega_i2c_busses[0]);
}
