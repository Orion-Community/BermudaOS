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
#include <sys/events/event.h>

#include <lib/spiram.h>
#include <lib/24c02.h>

#include <dev/dev.h>
#include <dev/spibus.h>
#include <dev/adc.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/usart/usart.h>
#include <dev/i2c/busses/atmega.h>
#include <dev/usart/busses/atmega_usart.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <arch/io.h>
#include <arch/adc.h>
#include <arch/avr/pgm.h>
#include <arch/avr/328/dev/spibus.h>

static VTIMER *timer;
static struct i2c_client slave_client;
static struct i2c_client master_client;

struct i2c_client eeprom_client;

#ifdef __THREADS__
static unsigned char i2c_slave_tx = 0x0;

static unsigned char i2c_slave_stack[128];
static uint8_t i2c_master_stack[128];

THREAD i2c_master;
THREAD i2c_slave;

static void slave_responder(struct i2c_message *msg)
{
	msg->buff = &i2c_slave_tx;
	msg->length = 1;
	i2c_slave_tx++;
	return;
}

THREAD(IIC_slave, arg)
{
	unsigned char rx = 0;
	int fd;
	
	atmega_i2c_init_client(&slave_client, ATMEGA_I2C_C0);
	slave_client.callback = &slave_responder;

	while(1) {
		fd = i2cdev_socket(&slave_client, _FDEV_SETUP_RW | I2C_SLAVE);
		if(fd < 0) {
			goto _usart;
		}
		i2cdev_listen(fd, &rx, 1);
		close(fd);
		
		_usart:

		printf_P(PSTR("RX=%X\n"), rx);
		BermudaThreadSleep(500);
	}

}

THREAD(TwiTest, arg)
{
	unsigned char tx[] = { 0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,
			     0xDC, 0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,
			     0x0 };
	int fd, rc;
	
	atmega_i2c_init_client(&master_client, ATMEGA_I2C_C0);
	master_client.sla = 0x54;
	master_client.freq = 100000UL;
	
	
	while(1) {
		fd = i2cdev_socket(&master_client, _FDEV_SETUP_RW | I2C_MASTER);
		if(fd < 0) {
			goto sleep;
		}
		
		rc = write(fd, tx, 20);
		rc += read(fd, NULL, 0);
		if(rc == 0) {
			flush(fd);
		} else {
			i2cdev_error(fd);
		}
		close(fd);

		tx[19]++;
		
		sleep:
		BermudaThreadSleep(1000);
	}
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
	printf_P(PSTR("Booting!\n"));
	BermudaSetPinMode(A0, INPUT);
	BermudaSetPinMode(5, OUTPUT);
#ifdef __THREADS__
	int fd;
	char buff[4];

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
	
	BermudaThreadCreate(&i2c_master, "TWI", &TwiTest, NULL, 128,
					i2c_master_stack, BERMUDA_DEFAULT_PRIO);
	BermudaThreadCreate(&i2c_slave, "IICS", &IIC_slave, NULL, 128, 
					i2c_slave_stack, BERMUDA_DEFAULT_PRIO);

#endif
	timer = BermudaTimerCreate(500, &TestTimer, NULL, BERMUDA_PERIODIC);
	atmega_i2c_init_client(&eeprom_client, ATMEGA_I2C_C0);
	Bermuda24c02Init(&eeprom_client);
	BermudaSpiRamInit(SPI0, 10);
	BermudaSpiRamWriteByte(0x50, 0xF8);
	Bermuda24c02WriteByte(100, 0xAC);
}

#ifdef __THREADS__
void loop()
#else
unsigned long loop()
#endif
{
	float tmp = 0;
	int temperature = 0;
	unsigned char read_back_eeprom = 0, read_back_sram = 0;
	
	tmp = ADC0->read(ADC0, A0, 500);
	temperature = tmp / 1024 * 5000;
	temperature /= 10;
	
	read_back_sram = BermudaSpiRamReadByte(0x50);
	read_back_eeprom = Bermuda24c02ReadByte(100);

	printf_P(PSTR("T=%u M=%X E=%X S=%X\n"), temperature, BermudaHeapAvailable(), read_back_eeprom,
				read_back_sram);

#ifdef __THREADS__
	BermudaThreadSleep(5000);
	return;
#else
	return 500;
#endif
}
