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

#include <sys/thread.h>
#include <sys/virt_timer.h>
#include <sys/events/event.h>

#include <lib/spiram.h>
#include <lib/24c02.h>

#include <dev/dev.h>
#include <dev/spibus.h>
#include <dev/usartif.h>
#include <dev/adc.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/busses/atmega.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <arch/io.h>
#include <arch/usart.h>
#include <arch/adc.h>
#include <arch/avr/328/dev/spibus.h>

static VTIMER *timer;
static struct i2c_client *slave_client;
static struct i2c_client master_client;

struct i2c_client eeprom_client;

#ifdef __THREADS__
// static unsigned char twi_slave_tx = 0x0;
// 
// 
// TWI_HANDLE(SlaveResponder, msg)
// {
// 	msg->tx_buff = (const void*)&twi_slave_tx;
// 	msg->tx_length = 1;
// 	msg->tmo = 500;
// 	twi_slave_tx++;
// }
// 
THREAD(SramThread, arg)
{
	char *buff[4];
// 	unsigned char rx = 0;
// 	int fd;
	
// 	client = BermudaHeapAlloc(sizeof(*client));
// 	atmega_i2c_init_client(client, ATMEGA_I2C_C0);
// 	client->sla = 0x54;
// 	client->freq = 100000UL;

	while(1) {
// // 		fd = i2cdev_socket(client, _FDEV_SETUP_RW | I2C_MASTER);
// // 		if(fd != -1) {
// // 			write(fd, &rx, 1);
// // 			read(fd, NULL, 0);
// // 			flush(fd);
// // 			close(fd);
// // 		}

		BermudaUsartListen(USART0, buff, 3, 9600, 0);
		buff[3] = '\0';
		BermudaPrintf("%s\n", buff);
	}
}

THREAD(TwiTest, arg)
{
	unsigned char tx[] = { 0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,
			     0xDC, 0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,0xAA, 0xAB, 0xAC,
			     0x0 };
	int fd;
	
	atmega_i2c_init_client(&master_client, ATMEGA_I2C_C0);
	master_client.sla = 0x54;
	master_client.freq = 100000UL;
	
	
	while(1) {
		fd = i2cdev_socket(&master_client, _FDEV_SETUP_RW | I2C_MASTER);
		if(fd < 0) {
			goto sleep;
		}
		
		write(fd, tx, 20);
		read(fd, NULL, 0);
		flush(fd);
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
	BermudaSetPinMode(A0, INPUT);
	BermudaSetPinMode(5, OUTPUT);
#ifdef __THREADS__
	BermudaThreadCreate(BermudaHeapAlloc(sizeof(THREAD)), "TWI", &TwiTest, NULL, 128,
					BermudaHeapAlloc(128), BERMUDA_DEFAULT_PRIO);
	BermudaThreadCreate(BermudaHeapAlloc(sizeof(THREAD)), "SRAM", &SramThread, NULL, 128, 
					BermudaHeapAlloc(128), BERMUDA_DEFAULT_PRIO);
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
	BermudaPrintf("The temperature is: %u :: Free mem: %X\n", temperature, BermudaHeapAvailable());
	
	read_back_sram = BermudaSpiRamReadByte(0x50);
	read_back_eeprom = Bermuda24c02ReadByte(100);

	BermudaPrintf("Read back value's: %X::%X\n", read_back_eeprom,
		read_back_sram);
	BermudaUsartTransfer(USART0, "USART output\r\n", 14, 9600, 500);
#ifdef __THREADS__
	BermudaThreadSleep(5000);
	return;
#else
	return 500;
#endif
}
