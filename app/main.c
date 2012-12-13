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
struct i2c_client eeprom_client;
static char i2c_stack[200];
THREAD i2c_thread;

static struct i2c_client *test_client = NULL;

#ifdef __THREADS__

THREAD(i2c_dbg, arg)
{
	uint8_t tx = 0, rx = 0;
	int fd;
	while(1) {
		fd = i2cdev_socket(test_client, _FDEV_SETUP_RW | I2C_MASTER);
		if(fd > 0) {
			write(fd, &tx, 1);
			read(fd, &rx, 1);
			flush(fd);
			close(fd);
		}
		printf("fd: %i\n", fd);
		BermudaThreadSleep(2000);
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
	
	i2c_init_client(&eeprom_client, ATMEGA_I2C_C0_ADAPTER, BASE_SLA_24C02, SCL_FRQ_24C02);
	test_client = i2c_alloc_client(ATMEGA_I2C_C0_ADAPTER, 0x48, 100000UL);
	BermudaThreadCreate(&i2c_thread, "I2C", &i2c_dbg, NULL, 200,
					&i2c_stack[0], BERMUDA_DEFAULT_PRIO);
#endif
	timer = BermudaTimerCreate(500, &TestTimer, NULL, BERMUDA_PERIODIC);
	Bermuda24c02Init(&eeprom_client);
	BermudaSpiRamInit(SPI0, 10);
// 	BermudaSpiRamWriteByte(0x50, 0xF8);
// 	Bermuda24c02WriteByte(100, 0xAC);
}

#ifdef __THREADS__
void loop()
#else
unsigned long loop()
#endif
{
	double tmp = 0;
	double temperature = 0;
	unsigned char read_back_eeprom = 0, read_back_sram = 0;
	
	tmp = ADC0->read(ADC0, A0, 500);
	temperature = tmp / 1024 * 5000;
	temperature /= 10;
	
	read_back_sram = BermudaSpiRamReadByte(0x50);
// 	read_back_eeprom = Bermuda24c02ReadByte(100);

	printf_P(PSTR("T=%f M=%X E=%X S=%X\n"), temperature, BermudaHeapAvailable(), read_back_eeprom,
				read_back_sram);
	
	
#ifdef __THREADS__
	BermudaThreadSleep(5000);
	return;
#else
	return 500;
#endif
}
