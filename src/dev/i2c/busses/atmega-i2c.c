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
static int atmega_i2c_init_transfer(FILE *stream);
static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres);
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq);
static int atmega_i2c_put_char(int c, FILE *stream);
static int atmega_i2c_get_char(FILE *stream);

static FDEV_SETUP_STREAM(i2c_c0_io, NULL, NULL, &atmega_i2c_put_char, 
						 &atmega_i2c_get_char, &atmega_i2c_init_transfer, 
						 I2C_FNAME, 0 /* flags */, NULL /* data */);

static struct atmega_i2c_priv i2c_c0 = {
	.twcr = &TWCR,
	.twdr = &TWDR,
	.twsr = &TWSR,
	.twar = &TWSR,
	.twbr = &TWBR,
	.twamr = &TWAMR,
	.isr = atmega_i2c_isr_handle,
};

#ifdef __THREADS__
static volatile void *bus_c0_mutex = SIGNALED;
static volatile void *bus_c0_master_queue = SIGNALED;
static volatile void *bus_c0_slave_queue = SIGNALED;
#endif

/**
 * \brief Array of all available I2C busses
 */
static struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

static struct i2c_message i2c_c0_msgs[I2C_MSG_NUM] = { {0}, {0}, {0}, {0}, };

/**
 * \brief Intialize an I2C bus.
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
	adapter->dev->mutex = &bus_c0_mutex;
	adapter->dev->ctrl = &atmega_i2c_ioctl;
	adapter->dev->io = &i2c_c0_io;
	
	adapter->dev->io->buff = (void*)i2c_c0_msgs;
	
	adapter->master_queue = &bus_c0_master_queue;
	adapter->slave_queue = &bus_c0_slave_queue;
	adapter->data = (void*)&i2c_c0;

	vfs_add(&i2c_c0_io);
	open(i2c_c0_io.name, _FDEV_SETUP_RW);
	
	sla |= B1;
	adapter->dev->ctrl(adapter->dev, I2C_IDLE, NULL);
	atmega_i2c_reg_write(i2c_c0.twar, &sla);
}

PUBLIC void atmega_i2c_init_client(struct i2c_client *client, uint8_t ifac)
{
	client->adapter = atmega_i2c_busses[ifac];
}

/**
 * \brief Start a master transfer.
 * \param stream The device I/O file.
 * 
 * This function is called by the user using the <b>flush(fd : int)</b> funtion.
 */
static int atmega_i2c_init_transfer(FILE *stream)
{
	auto void config_bitrate();
	struct i2c_client *client = stream->data;
	struct i2c_message *msgs = (struct i2c_message*)stream->buff;
	struct i2c_adapter *adap = client->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t pres, twbr;
	int rc;
	
	if((stream->flags & I2C_MASTER) == I2C_MASTER) {
		config_bitrate();
		adap->dev->ctrl(adap->dev, I2C_START, NULL);
#ifdef __THREADS__
			rc = BermudaEventWaitNext( (volatile THREAD**)adap->master_queue, I2C_TMO);
#else
			adapter->master_queue = 1;
			BermudaIoWait(&(adapter->master_queue));
			rc = 0;
#endif
	} else {
		if(msgs[I2C_MASTER_TRANSMIT_MSG].length || msgs[I2C_MASTER_RECEIVE_MSG].length) {
			adap->dev->ctrl(adap->dev, I2C_START, NULL);
		} else {
			adap->dev->ctrl(adap->dev, I2C_LISTEN, NULL);
		}
#ifdef __THREADS__
			rc = BermudaEventWaitNext( (volatile THREAD**)adap->slave_queue, I2C_TMO);
#else
			adapter->slave_queue = 1;
			BermudaIoWait(&(adapter->slave_queue));
			rc = 0;
#endif
	}
	
	return rc;
	
	auto void config_bitrate()
	{
		pres = atmega_i2c_calc_prescaler(client->freq);
		twbr = atmega_i2c_calc_twbr(client->freq, pres);
		atmega_i2c_set_bitrate(priv, twbr, pres);
	}
}

static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data)
{
	struct i2c_adapter *adap = dev->ioctl;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t reg = 0;
	
	atmega_i2c_reg_read(priv->twcr, &reg);
	
	switch((uint16_t)cfg) {
		/* transaction control */
		case I2C_START:
			reg |= BIT(TWSTA) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
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
		case I2C_ACK:
			reg = (reg & ~BIT(TWSTA)) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_NACK:
			reg = (reg & ~(BIT(TWEA | BIT(TWSTA)))) | BIT(TWINT) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_BLOCK:
			reg |= (reg & ~BIT(TWINT)) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
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
	
	twbr = (F_CPU - (16*freq)) / (2*prescaler*freq);
	
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

static int atmega_i2c_put_char(int c, FILE *stream)
{
	struct i2c_adapter *adap = ((struct i2c_client*)stream->data)->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t data = (uint8_t)c&0xFF;
	atmega_i2c_reg_write(priv->twdr, &data);
	
	return c;
}

static int atmega_i2c_get_char(FILE *stream)
{
	struct i2c_adapter *adap = ((struct i2c_client*)stream->data)->adapter;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t data = 0;
	
	atmega_i2c_reg_read(priv->twdr, &data);
	
	return (int)(data&0xFF);
}

ISR(atmega_i2c_isr_handle, adapter, struct i2c_adapter *)
{
	struct atmega_i2c_priv *priv = (struct atmega_i2c_priv*)adapter->data;
	struct device *dev = adapter->dev;
	struct i2c_message *msgs = (struct i2c_message*)dev->io->buff;
	uint8_t status = atmega_i2c_get_status(priv);
	int fd, i;
	
	if(adapter->dev->io->fd < 0) {
		return;
	} else {
		fd = adapter->dev->io->fd;
	}
	
	switch(status) {
		case I2C_MASTER_START:
		case I2C_MASTER_REP_START:
			atmega_i2c_reset_index(fd);
			adapter->flags = I2C_MASTER_ENABLE;
			
			if(msgs[I2C_MASTER_TRANSMIT_MSG].length) {
				adapter->flags |= I2C_TRANSMITTER;
				msgs[I2C_MASTER_TRANSMIT_MSG].addr &= I2C_SLA_WRITE_MASK;
				fdputc(msgs[I2C_MASTER_TRANSMIT_MSG].addr, fd);
			} else {
				adapter->flags |= I2C_RECEIVER;
				msgs[I2C_MASTER_RECEIVE_MSG].addr |= I2C_SLA_READ_BIT;
				fdputc(msgs[I2C_MASTER_RECEIVE_MSG].addr, fd);
			}

			dev->ctrl(dev, I2C_ACK, NULL);
			break;
			
		case I2C_MT_SLA_ACK:
		case I2C_MT_DATA_ACK:
			if(atmega_i2c_get_index(fd) < msgs[I2C_MASTER_TRANSMIT_MSG].length) {
				fdputc((int)msgs[I2C_MASTER_TRANSMIT_MSG].buff[atmega_i2c_get_index(fd)], fd);
				atmega_i2c_inc_index(fd);
				dev->ctrl(dev, I2C_ACK, NULL);
			} else if(msgs[I2C_MASTER_RECEIVE_MSG].length) {
				adapter->flags &= ~I2C_TRANSMITTER;
				adapter->flags |= I2C_RECEIVER;
				msgs[I2C_MASTER_TRANSMIT_MSG].length = 0;
				dev->ctrl(dev, I2C_START, NULL); /* sent the repeated start */
			} else {
				msgs[I2C_MASTER_TRANSMIT_MSG].length = 0;

				if(msgs[I2C_SLAVE_RECEIVE_MSG].length) {
					adapter->flags = 0;
					adapter->flags |= I2C_RECEIVER;
					dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
				} else {
					adapter->flags = 0;
					dev->ctrl(dev, I2C_STOP | I2C_IDLE, NULL);
				}
#ifdef __THREADS__
					BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
					BermudaIoSignal(&(adapter->master_queue));
#endif
			}
			break;
			
		case I2C_MT_SLA_NACK:
		case I2C_MT_DATA_NACK:
			msgs[I2C_MASTER_TRANSMIT_MSG].length = 0;
			/* roll through */
		case I2C_MR_SLA_NACK:
		case I2C_MASTER_ARB_LOST:
			msgs[I2C_MASTER_RECEIVE_MSG].length = 0;
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
#endif
			
			if(status == I2C_MASTER_ARB_LOST) {
				dev->ctrl(dev, I2C_RELEASE, NULL);
				adapter->flags = 0;
				break;
			}
			
			if(msgs[I2C_SLAVE_RECEIVE_MSG].length) {
				adapter->flags = 0;
				adapter->flags |= I2C_RECEIVER;
				dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
			} else {
				adapter->flags = 0;
				dev->ctrl(dev, I2C_STOP | I2C_IDLE, NULL);
			}
			break;
			
		case I2C_MR_DATA_ACK:
			if(atmega_i2c_get_index(fd) < msgs[I2C_MASTER_RECEIVE_MSG].length) {
				msgs[I2C_MASTER_RECEIVE_MSG].buff[atmega_i2c_get_index(fd)] = (uint8_t)fdgetc(fd);
				atmega_i2c_inc_index(fd);
			}
			/* fall through to sla ack */
		case I2C_MR_SLA_ACK:
			if(atmega_i2c_get_index(fd) + 1 < msgs[I2C_MASTER_RECEIVE_MSG].length) {
				/*
				 * ACK if there is more than 1 byte to transfer.
				 */
				dev->ctrl(dev, I2C_ACK, NULL);
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
			}
			break;
			
		case I2C_MR_DATA_NACK:
			if(atmega_i2c_get_index(fd) < msgs[I2C_MASTER_RECEIVE_MSG].length) {
				msgs[I2C_MASTER_RECEIVE_MSG].buff[atmega_i2c_get_index(fd)] = (uint8_t)fdgetc(fd);
			}
			
			msgs[I2C_MASTER_RECEIVE_MSG].length = 0;
			
			if(msgs[I2C_SLAVE_RECEIVE_MSG].length) {
				dev->ctrl(dev, I2C_LISTEN | I2C_STOP, NULL);
				adapter->flags = 0;
				adapter->flags |= I2C_RECEIVER;
			} else {
				dev->ctrl(dev, I2C_IDLE | I2C_STOP, NULL);
				adapter->flags = 0;
			}
#ifdef __THREADS__
			BermudaEventSignalFromISR( (volatile THREAD**)adapter->master_queue);
#else
			BermudaIoSignal(&(adapter->master_queue));
#endif
			
			break;
		default:
			
			for(i = 0; i < I2C_MSG_NUM; i++) {
				msgs[i].buff = NULL;
				msgs[i].length = 0;
			}
			
			atmega_i2c_reset_index(fd);
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
