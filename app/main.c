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

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/virt_timer.h>
#include <sys/events/event.h>

#include <lib/24c02.h>
#include <lib/spiram.h>

#include <dev/dev.h>
#include <dev/twif.h>
#include <dev/spibus.h>
#include <dev/usartif.h>

#include <arch/io.h>
#include <arch/twi.h>
#include <arch/usart.h>
#include <arch/avr/328/dev/spibus.h>

static VTIMER *timer;

THREAD(SramThread, arg)
{
	unsigned char rx = 0, tx = 0xDB;
	unsigned int num = 0;
	char *buff[4];
	
	while(1) {
		int rc = BermudaTwiSlaveListen(TWI0, &num, &rx, 1, 1000);
		if(rc == 0) {
			BermudaTwiSlaveRespond(TWI0, &tx, 1, 500);
		}
		BermudaPrintf("NUM: 0x%X :: RX: 0x%X :: rc: %i\n", num, rx, rc);
		BermudaUsartListen(USART0, buff, 3, 9600, 500);
		buff[3] = '\0';
		BermudaPrintf("%s\n", buff);
	}
}

static unsigned char led = 1;

PUBLIC void TestTimer(VTIMER *timer, void *arg)
{
	BermudaDigitalPinWrite(5, led);
	led ^= 1;
}

int main(void)
{
	BermudaInit();

	while(1);
	return 0;
}

void setup()
{
	BermudaSetPinMode(A0, INPUT);
	BermudaSetPinMode(5, OUTPUT);
	BermudaThreadCreate(BermudaHeapAlloc(sizeof(THREAD)), "SRAM", &SramThread, NULL, 128, 
					BermudaHeapAlloc(128), BERMUDA_DEFAULT_PRIO);
	timer = BermudaTimerCreate(500, &TestTimer, NULL, BERMUDA_PERIODIC);

	Bermuda24c02Init(TWI0);
	BermudaSpiRamInit(SPI0, 10);
	BermudaSpiRamWriteByte(0x50, 0xF8);
	Bermuda24c02WriteByte(100, 0xAC);
}

void loop()
{
	struct adc *adc = BermudaGetADC();
	float tmp = 0;
	int temperature = 0;
	unsigned char read_back_eeprom = 0, read_back_sram = 0;
	char *buff[10];

	tmp = adc->read(A0);
	temperature = tmp / 1024 * 5000;
	temperature /= 10;
	BermudaPrintf("The temperature is: %u :: Free mem: %X\n", temperature, BermudaHeapAvailable());

	read_back_sram = BermudaSpiRamReadByte(0x50);
	read_back_eeprom = Bermuda24c02ReadByte(100);

	BermudaPrintf("Read back value's: %X::%X\n", read_back_eeprom,
		read_back_sram);
	BermudaUsartTransfer(USART0, "USART output\r\n", 14, 9600, 500);
	BermudaThreadSleep(5000);
	return;
}
