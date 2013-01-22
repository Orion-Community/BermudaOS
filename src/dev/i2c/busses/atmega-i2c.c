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
#include <dev/i2c.h>
#include <dev/i2c-reg.h>
#include <dev/i2c-core.h>
#include <dev/i2c-msg.h>
#include <dev/i2c/busses/atmega.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <fs/vfile.h>

#include <arch/io.h>
#include <arch/irq.h>

#include "atmega_priv.h"

/* static functions */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data);
static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres);
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq);

static int i2c_init_transfer(struct i2c_adapter *adap, uint32_t freq, bool master, size_t *index);
static int i2c_resume_transfer(struct i2c_adapter *adapter, size_t *index);
static int i2c_master_transfer(struct i2c_adapter *adapter, uint32_t);
static int atmega_i2c_slave_listen(struct i2c_adapter *adapter, size_t *index);
static void atmega_i2c_update(struct i2c_adapter *adapter, long diff);
static void atmega_i2c_slave_buff_end(struct i2c_adapter *adapter);

#ifdef I2C_DBG
static void atmega_i2c_dbg(struct i2c_adapter *adapter);
#endif

/**
 * \brief File I/O structure for bus 0 on port C.
 */
static FDEV_SETUP_STREAM(i2c_c0_io, NULL, NULL, NULL, 
						 NULL, NULL, 
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
struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

/**
 * \brief Buffer index
 */
static volatile size_t buffer_index = 0;

/**
 * \brief Master message which is currently configured for transfer.
 */
static volatile struct i2c_message *master_msg = NULL;
/**
 * \brief Master message which is sent/received last by the controller.
 */
static volatile struct i2c_message *last_master_msg = NULL;
/**
 * \brief Slave message which is currently configured for transfer.
 */
static volatile struct i2c_message *slave_msg = NULL;
/**
 * \brief Last used slave message.
 */
static volatile struct i2c_message *last_slave_msg = NULL;

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

#ifdef __THREADS__
	adapter->master_queue = &bus_c0_master_queue;
	adapter->slave_queue = &bus_c0_slave_queue;
#else
	adapter->mutex = 0;
	adapter->master_queue = 1;
	adapter->slave_queue = 1;
#endif
	adapter->data = (void*)&i2c_c0;
	adapter->features = I2C_MASTER_SUPPORT | I2C_SLAVE_SUPPORT;
	adapter->xfer = &i2c_init_transfer;
	adapter->resume = &i2c_resume_transfer;
	adapter->update = &atmega_i2c_update;
	adapter->busy = FALSE;
	adapter->error = FALSE;

	vfs_add(&i2c_c0_io);
	open(i2c_c0_io.name, _FDEV_SETUP_RW);
	
	atmega_i2c_reg_write(i2c_c0.twar, &sla);
	adapter->dev->ctrl(adapter->dev, I2C_IDLE, NULL);
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
			
		case I2C_RELEASE | I2C_ACK:
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
 * \brief Initiate an I2C transfer.
 * \param adapter Bus adapter.
 * \param freq Operating frequency.
 * \param master 
 * \note \p freq will only be used if a master transfer is being initiated.
 */
static int i2c_init_transfer(struct i2c_adapter *adapter, uint32_t freq, bool master, size_t *index)
{
	struct i2c_message *msg;
	int rc;
	
	adapter->error = FALSE;
	if(master) {
		rc = i2c_master_transfer(adapter, freq);
		BermudaEnterCritical();
		*index = i2c_vector_locate(adapter, (void*)last_master_msg);
		msg = (struct i2c_message*)last_master_msg;
		BermudaExitCritical();
		if((msg->features & I2C_MSG_CALL_BACK_MASK) != 0) {
			if(rc != 0) {
				adapter->dev->ctrl(adapter->dev, I2C_RELEASE | I2C_ACK, NULL);
				msg->features |= I2C_MSG_DONE_FLAG;
				adapter->busy = FALSE;
			}
		} else {
			rc = 1;
		}
	} else {
		/* slave msg */
		rc = atmega_i2c_slave_listen(adapter, index);
	}
	return rc;
}

/**
 * \brief Initialize a master transfer.
 * \param adapter Bus adapter.
 * \param Desired frequency.
 *
 * This function will set call atmega_i2c_update to set master_msg and slave_msg (if the bus
 * is not busy yet).
 */
static int i2c_master_transfer(struct i2c_adapter *adapter, uint32_t freq)
{
	int rc = -1;
	uint8_t pres = atmega_i2c_calc_prescaler(freq);
	uint8_t twbr = atmega_i2c_calc_twbr(freq, pres);
	
	TWBR = twbr;
	TWSR &= ~B11;
	TWSR |= pres & B11;
	
	if(!adapter->busy) {
		if((TWSR & I2C_NOINFO) == I2C_NOINFO) {
			atmega_i2c_update(adapter, 0); /* will be called by the ISR if the bus is busy */
			if(slave_msg) {
				adapter->dev->ctrl(adapter->dev, I2C_START | I2C_ACK, NULL);
			} else {
				adapter->dev->ctrl(adapter->dev, I2C_START | I2C_NACK, NULL);
			}
		}
		
	}
	rc = BermudaEventWaitNext(event(adapter->master_queue), I2C_MASTER_TMO); /* wait for completion */
	return rc;
}

/**
 * \brief Setup the slave buffers.
 * \param adapter Bus adapter.
 * \param index Index of the first slave message.
 */
static int atmega_i2c_slave_listen(struct i2c_adapter *adapter, size_t *index)
{
	int rc = 0;
	
	if(!adapter->busy) {
		if((TWSR & I2C_NOINFO) == I2C_NOINFO) {
			atmega_i2c_update(adapter, 0); /* Will be called by the ISR if the bus is busy. */
			if(master_msg) {
				adapter->dev->ctrl(adapter->dev, I2C_START | I2C_ACK, NULL);
			} else {
				adapter->dev->ctrl(adapter->dev, I2C_LISTEN, NULL);
			}
		}
	}
	
	if(BermudaEventWaitNext(event(adapter->slave_queue), I2C_SLAVE_TMO) == -1) {
		rc = -1;
		if(slave_msg) {
			slave_msg->features |= I2C_MSG_DONE_FLAG;
			if((i2c_msg_features((void*)slave_msg) & I2C_MSG_CALL_BACK_MASK) != 0) {
				adapter->busy = FALSE;
				adapter->dev->ctrl(adapter->dev, I2C_RELEASE | I2C_ACK, NULL);
			}
		}
	}
	
	*index = i2c_vector_locate(adapter, (void*)last_slave_msg);
	
	return rc;
}

/**
 * \brief Resume transmission after a call back.
 * \param adapter Bus adapter.
 * \param index Pointer to the index of the inserted message.
 * \see i2c_adapter::resume
 * 
 * In case of master message, master_msg will be set to the message located at \code *index \endcode
 * If the message at \p *index is a slave message, slave_msg will be set to \p *index.
 */
static int i2c_resume_transfer(struct i2c_adapter *adapter, size_t *index)
{
	struct i2c_message *msg;
	int rc = 1;
	adapter->error = FALSE;
	
	msg = i2c_vector_get(adapter, *index);
	if((neg(i2c_msg_features(msg)) & I2C_MSG_MASTER_MSG_MASK) != 0) {
		master_msg = msg;
		adapter->dev->ctrl(adapter->dev, I2C_START | I2C_NACK, NULL);
		if((rc = BermudaEventWaitNext(event(adapter->master_queue), I2C_MASTER_TMO)) < 0) {
			adapter->error = TRUE;
		}
		*index = i2c_vector_locate(adapter, (void*)last_master_msg);
		msg = (struct i2c_message*)last_master_msg;
		if((msg->features & I2C_MSG_CALL_BACK_MASK) != 0) {
			if(rc != 0) {
				adapter->busy = FALSE;
				adapter->dev->ctrl(adapter->dev, I2C_RELEASE | I2C_ACK, NULL);
				msg->features |= I2C_MSG_DONE_FLAG;
			}
		} else {
			rc = 1;
		}
	} else {
		/* slave msg */
		slave_msg = msg;
		adapter->dev->ctrl(adapter->dev, I2C_RELEASE | I2C_ACK, NULL);
		if((rc = BermudaEventWaitNext(event(adapter->slave_queue), I2C_SLAVE_TMO)) < 0) {
			rc = -1;
			msg = (struct i2c_message*)last_slave_msg;
			if(msg) {
				msg->features |= I2C_MSG_DONE_FLAG;
				if((i2c_msg_features(msg) & I2C_MSG_CALL_BACK_MASK) != 0) {
					adapter->busy = FALSE;
					adapter->dev->ctrl(adapter->dev, I2C_RELEASE | I2C_ACK, NULL);
				}
			}
		}
		*index = i2c_vector_locate(adapter, (void*)last_slave_msg);
	}
	
	return rc;
}

/**
 * \brief Update the bus message index.
 * \param diff The index diff.
 * \see i2c_adapter::update i2c_update
 */
static void atmega_i2c_update(struct i2c_adapter *adapter, long diff)
{
	size_t index = 0;
	
	BermudaEnterCritical();
	if(i2c_first_master_msg(adapter, &index)) {
		master_msg = i2c_vector_get(adapter, index);
	} else {
		master_msg = NULL;
	}
	
	if(i2c_first_slave_msg(adapter, &index)) {
		slave_msg = i2c_vector_get(adapter, index);
	} else {
		slave_msg = NULL;
	}
	BermudaExitCritical();
}

static void atmega_i2c_slave_buff_end(struct i2c_adapter *adapter)
{
	struct device *dev = adapter->dev;
	i2c_features_t features;
	
	if(!slave_msg) {
		adapter->busy = FALSE;
		atmega_i2c_update(adapter, 0);
		if(master_msg) {
			dev->ctrl(dev, I2C_START | I2C_NACK, NULL);
		} else {
			dev->ctrl(dev, I2C_NACK, NULL);
		}
		event_signal_from_isr(event(adapter->slave_queue));
	}
	
	features = i2c_msg_features((void*)slave_msg);
	last_slave_msg = slave_msg;
	
	if((features & I2C_MSG_CALL_BACK_MASK) != 0) {
		event_signal_from_isr(event(adapter->slave_queue));
		dev->ctrl(dev, I2C_BLOCK, NULL);
		adapter->error = 0;
	} else {
		features |= I2C_MSG_DONE_FLAG;
		slave_msg->features = features;
		atmega_i2c_update(adapter, 0);
		if(slave_msg) {
			dev->ctrl(dev, I2C_LISTEN, NULL);
		} else if(master_msg) {
			dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
			event_signal_from_isr(event(adapter->slave_queue));
			adapter->busy = FALSE;
		} else {
			event_signal_from_isr(event(adapter->slave_queue));
			dev->ctrl(dev, I2C_NACK, NULL);
			adapter->busy = FALSE;
		}
	}
}

#ifdef I2C_DBG
static void atmega_i2c_dbg(struct i2c_adapter *adapter)
{
	struct i2c_message *msg;
	i2c_features_t features;
	
	i2c_vector_foreach(&adapter->msg_vector, i) {
		msg = i2c_vector_get(adapter, i);
		features = i2c_msg_features(msg);
		features |= I2C_MSG_DONE_FLAG;
		msg->features = features;
		master_msg_index++;
	}
}
#endif

#ifndef __DOXYGEN__
SIGNAL(TWI_STC_vect)
{
	uint8_t status = TWSR & I2C_NOINFO, twcr = TWCR;
	struct i2c_adapter *adapter = ATMEGA_I2C_C0_ADAPTER;
	struct device *dev = adapter->dev;
	
	switch(status) {
		/*
		 * initiate the master transfer.
		 */
		case I2C_MASTER_START:
		case I2C_MASTER_REP_START:
			buffer_index = 0;
			adapter->busy = TRUE;
			adapter->error = FALSE;
			TWDR = master_msg->addr;
			TWCR = BIT(TWEN) | BIT(TWIE) | (twcr & BIT(TWEA)) | BIT(TWINT);
			break;
			
		case I2C_MT_SLA_ACK:
		case I2C_MT_DATA_ACK:
			if(buffer_index < master_msg->length) {
				TWDR = master_msg->buff[buffer_index];
				buffer_index++;
				dev->ctrl(dev, I2C_ACK, NULL);
				break;
			}
			
			last_master_msg = master_msg;
			if((master_msg->features & I2C_MSG_CALL_BACK_MASK) != 0) {
				event_signal_from_isr(event(adapter->master_queue));
				dev->ctrl(dev, I2C_BLOCK, NULL);
				break;
			}
			
			master_msg->features |= I2C_MSG_DONE_FLAG;
			atmega_i2c_update(adapter, 0);
			
			if(master_msg) {
				dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
				break;
			} else if(slave_msg) {
				adapter->busy = FALSE;
				dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
				event_signal_from_isr(event(adapter->master_queue));
				break;
			} else {
				adapter->busy = FALSE;
				dev->ctrl(dev, I2C_STOP | I2C_NACK, NULL);
				event_signal_from_isr(event(adapter->master_queue));
				break;
			}
			
		case I2C_MT_SLA_NACK:
		case I2C_MT_DATA_NACK:
		case I2C_MR_SLA_NACK:
			master_msg->features |= I2C_MSG_DONE_FLAG;
			last_master_msg = master_msg;
			atmega_i2c_update(adapter, 0);
			adapter->busy = FALSE;
			event_signal_from_isr(event(adapter->master_queue));
			
			if(slave_msg) {
				dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
			} else {
				dev->ctrl(dev, I2C_STOP | I2C_NACK, NULL);
			}
			break;

		case I2C_MASTER_ARB_LOST:
			adapter->busy = FALSE;
			TWCR = BIT(TWINT) | BIT(TWEN) | BIT(TWIE) | (twcr & BIT(TWEA)) | BIT(TWSTA);
			break;
			
		/*
		 * Master receiver.
		 */
		case I2C_MR_DATA_ACK:
			if(buffer_index < master_msg->length) {
				master_msg->buff[buffer_index] = TWDR;
				buffer_index++;
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
				break;
			}
		case I2C_MR_SLA_ACK:
			if((buffer_index+1) < master_msg->length) {
				dev->ctrl(dev, I2C_ACK, NULL);
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
			}
			break;
			
		case I2C_MR_DATA_NACK:
			if(buffer_index < master_msg->length) {
				master_msg->buff[buffer_index] = TWDR;
			}
			
			last_master_msg = master_msg;
			if((master_msg->features & I2C_MSG_CALL_BACK_MASK) != 0) {
				event_signal_from_isr(event(adapter->master_queue));
				dev->ctrl(dev, I2C_BLOCK, NULL);
				break;
			}
			
			master_msg->features |= I2C_MSG_DONE_FLAG;
			atmega_i2c_update(adapter, 0);
			if(master_msg) {
				dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
				break;
			} else if(slave_msg) {
				dev->ctrl(dev, I2C_STOP | I2C_LISTEN, NULL);
			} else {
				dev->ctrl(dev, I2C_STOP | I2C_NACK, NULL);
			}
			adapter->busy = FALSE;
			event_signal_from_isr(event(adapter->master_queue));
			break;
		
		case I2C_SR_GC_ARB_LOST:
		case I2C_SR_SLAW_ARB_LOST:
		case I2C_SR_SLAW_ACK:
		case I2C_SR_GC_ACK:
			buffer_index = 0;

			if(slave_msg) {
				if(slave_msg->length) {
					dev->ctrl(dev, I2C_ACK, NULL);
					adapter->busy = TRUE;
					slave_msg->addr = TWDR;
					break;
				} else {
					dev->ctrl(dev, I2C_NACK, NULL);
				}
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
			}
			break;
			
		case I2C_SR_SLAW_DATA_ACK:
		case I2C_SR_GC_DATA_ACK:
			if(!slave_msg) {
				dev->ctrl(dev, I2C_NACK, NULL);
				break;
			}
			
			if(buffer_index < slave_msg->length) {
				slave_msg->buff[buffer_index] = TWDR;
				buffer_index++;
			} else {
				slave_msg->length = 0;
			}
			
			if(slave_msg->length) {
				dev->ctrl(dev, I2C_ACK, NULL);
				break;
			}
			
		case I2C_SR_SLAW_DATA_NACK:
		case I2C_SR_GC_DATA_NACK:
			if(master_msg) {
				dev->ctrl(dev, I2C_START | I2C_ACK, NULL);
			} else {
				dev->ctrl(dev, I2C_NACK, NULL);
			}
			break;
			
		case I2C_SR_STOP:
			if(*(adapter->slave_queue) == NULL || *(adapter->slave_queue) == SIGNALED) {
				atmega_i2c_update(adapter, 0);
				if(master_msg) {
					dev->ctrl(dev, I2C_START | I2C_NACK, NULL);
				} else {
					dev->ctrl(dev, I2C_NACK, NULL);
				}
				if(slave_msg) {
					slave_msg->features |= I2C_MSG_DONE_FLAG;
				}
				last_slave_msg = NULL;
				adapter->busy = FALSE;
				adapter->error = FALSE;
			} else {
				atmega_i2c_slave_buff_end(adapter);
			}
			break;
		
		case I2C_ST_ARB_LOST:
		case I2C_ST_SLAR_ACK:
			adapter->busy = TRUE;
			buffer_index = 0;
			
		case I2C_ST_DATA_ACK:
			if(slave_msg) {
				if(slave_msg->length && buffer_index < slave_msg->length) {
					TWDR = slave_msg->buff[buffer_index];
					buffer_index++;
					if(buffer_index < slave_msg->length) {
						dev->ctrl(dev, I2C_ACK, NULL);
						break;
					}
				} else {
					TWDR = 0;
				}
			} else {
				TWDR = 0;
			}
			dev->ctrl(dev, I2C_NACK, NULL);
			break;
			
		case I2C_ST_DATA_NACK:
		case I2C_ST_LAST_DATA_ACK:
			atmega_i2c_slave_buff_end(adapter);
			break;
			
		default:
			dev->ctrl(dev, I2C_STOP | I2C_NACK, NULL);
			event_signal_from_isr(event(adapter->slave_queue));
			event_signal_from_isr(event(adapter->master_queue));
			adapter->busy = FALSE;
			adapter->error = TRUE;
			break;
	}
}
#endif
