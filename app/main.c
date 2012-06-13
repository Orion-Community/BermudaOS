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

#include <dev/dev.h>
#include <dev/twif.h>

#include <arch/io.h>
#include <arch/twi.h>

static VTIMER *timer;

THREAD(SramThread, arg)
{
	while(1) {
		//printf("Read back: %x\n", 0);
		BermudaThreadSleep(1000);
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
	//BermudaSpiRamInit(SPI0, 10);
	//BermudaSpiRamWriteByte(0x50, 0x99);
	Bermuda24c02Init(TWI0);
}

void loop()
{
	struct adc *adc = BermudaGetADC();
	float tmp = 0;
	int temperature = 0;
             
	tmp = adc->read(A0);
	temperature = tmp / 1024 * 5000;
	temperature /= 10;
	//printf("The temperature is: %u :: Free mem: %X\n", temperature, BermudaHeapAvailable());

	BermudaThreadSleep(5000);
	return;
}
