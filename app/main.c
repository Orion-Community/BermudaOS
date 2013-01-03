/*
 *  BermudaOS - App main
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
#include <string.h>

#include <sys/thread.h>
#include <sys/virt_timer.h>
#include <sys/epl.h>
#include <sys/events/event.h>

#include <lib/spiram.h>
#include <lib/24c02.h>

#include <dev/dev.h>
#include <dev/spibus.h>
#include <dev/adc.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/i2c/i2c-core.h>
#include <dev/i2c/i2c-msg.h>
#include <dev/usart/usart.h>
#include <dev/i2c/busses/atmega.h>
#include <dev/usart/busses/atmega_usart.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <arch/io.h>
#include <arch/adc.h>
#include <arch/avr/pgm.h>
#include <arch/avr/328/dev/spibus.h>

#include <net/core/dev.h>

static VTIMER *timer;
struct i2c_client *eeprom_client;
static unsigned char __link i2c_stack[150];
static unsigned char __link i2c_slave_stack[150];
THREAD i2c_thread;
THREAD i2c_slave_thread;

static struct i2c_client *test_client = NULL, *test_client2 = NULL;

static uint8_t test_tx[2] = { 0xFC, 0xAA };

#ifdef __THREADS__
THREAD(i2c_dbg, arg)
{
	int fd;
	uint8_t tx = 0xAB;
	while(1) {
		
		fd = i2cdev_socket(test_client2, _FDEV_SETUP_RW | I2C_MASTER | I2CDEV_CALL_BACK);
		i2c_set_transmission_layout(test_client, "ww");
		
		if(fd >= 0) {
			write(fd, &tx, 1);
			flush(fd);
			close(fd);
		}
		tx++;
		BermudaThreadSleep(1000);
	}
}

THREAD(i2c_slave_dbg, arg)
{
	int slave;
	uint8_t rx = 0;
	while(1)
	{
		slave = i2cdev_socket(test_client, _FDEV_SETUP_RW | I2C_SLAVE | I2CDEV_CALL_BACK);
		if(slave >= 0) {
			i2cdev_listen(slave, &rx, 1);
			close(slave);
		}
		printf_P(PSTR("rx: %X\n"), rx);
	}
}

static int master_callback(struct i2c_client *client, struct i2c_message *msg)
{
	msg->buff = &test_tx[0];
	msg->length = 1;
	msg->addr = 0x54;
	msg->features = I2C_MSG_MASTER_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG | I2C_MSG_SENT_STOP_FLAG;
// 	printf("hi");
	test_tx[0]++;
	return 0;
}

static int slave_callback(struct i2c_client *client, struct i2c_message *msg)
{
	msg->buff = &test_tx[1];
	msg->length = 1;
	msg->addr = 0x54;
	msg->features = I2C_MSG_SLAVE_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG;
// 	printf("hi");
	test_tx[1]++;;
	return 0;
}
#endif

static unsigned char led = 1;
PUBLIC void TestTimer(VTIMER *timer, void *arg)
{
	BermudaDigitalPinWrite(5, led);
	led ^= 1;
}

void setup()
{

}

static char buff[4];
void app()
{
	printf_P(PSTR("Booting!\n"));
	BermudaSetPinMode(A0, INPUT);
	BermudaSetPinMode(5, OUTPUT);
#ifdef __THREADS__
	int fd;
	
	while(1) {
		fd = usartdev_socket(USART0, "USART0", _FDEV_SETUP_RW);
		if(fd < 0) {
			_exit();
		}
		read(fd, buff, 3);
		usartdev_close(fd);
		buff[3] = '\0';
		if(!strcmp(buff, "run")) {
			break;
		}
		BermudaThreadSleep(500);
	}
	
	eeprom_client = i2c_alloc_client(ATMEGA_I2C_C0_ADAPTER, BASE_SLA_24C02, SCL_FRQ_24C02);
	test_client = i2c_alloc_client(ATMEGA_I2C_C0_ADAPTER, 0x54, 100000UL);
	test_client2 = i2c_alloc_client(ATMEGA_I2C_C0_ADAPTER, 0x54, 100000UL);
	
	i2c_set_callback(test_client, &slave_callback);
	i2c_set_callback(test_client2, &master_callback);
#endif

	timer = BermudaTimerCreate(500, &TestTimer, NULL, BERMUDA_PERIODIC);
	BermudaSpiRamInit(SPI0, 10);
	Bermuda24c02Init(eeprom_client);
	BermudaSpiRamWriteByte(0x50, 0xF8);
	BermudaThreadCreate(&i2c_thread, "I2CTH", &i2c_dbg, NULL, 150,
					&i2c_stack[0], BERMUDA_DEFAULT_PRIO);
	BermudaThreadCreate(&i2c_slave_thread, "I2C_SLAVE", &i2c_slave_dbg, NULL, 150,
					&i2c_slave_stack[0], BERMUDA_DEFAULT_PRIO);
	
	unsigned char tx_eep = 0, rx_eep = 0;
	while(1) {
		float tmp = 0;
		uint8_t read_back_sram = 0;
		
		tmp = ADC0->read(ADC0, A0, 500);
		tmp = tmp / 1024 * 5000;
		tmp /= 10;
		
		Bermuda24c02WriteByte(100, ++tx_eep);
		BermudaDelay(12);
		read_back_sram = BermudaSpiRamReadByte(0x50);
		rx_eep = Bermuda24c02ReadByte(100);

		printf_P(PSTR("T=%f M=%X E=%X S=%X L=%u\n"), tmp, BermudaHeapAvailable(), 
				 rx_eep, read_back_sram, i2c_vector_length(ATMEGA_I2C_C0_ADAPTER));
		BermudaThreadSleep(5000);
	}
}
