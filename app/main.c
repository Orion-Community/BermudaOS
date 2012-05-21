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
#include <arch/io.h>
#include <lib/spiram.h>
#include <stdlib.h>

THREAD(TemperatureThread, arg)
{
	struct adc *adc = BermudaGetADC();
	float tmp = 0;
	int temperature = 0;

	while(1)
	{
		tmp = adc->read(A0);
		temperature = tmp / 1024 * 5000;
		temperature /= 10;
		printf("The temperature is: %u :: Free mem: %X\n", temperature, BermudaHeapAvailable());
		BermudaThreadSleep(500);
	}
}

int main(void)
{
	BermudaInit();

	while(1);
	return 0;
}

void setup()
{
	BermudaSpiRamInit();
	BermudaSpiRamWriteByte(0x58, 0x99);
	BermudaSetPinMode(A0, INPUT);
	BermudaThreadCreate(BermudaHeapAlloc(sizeof(THREAD)), "TEMP TH", &TemperatureThread, NULL, 64, 
			BermudaHeapAlloc(64), BERMUDA_DEFAULT_PRIO);
}

void loop()
{
	unsigned char data = BermudaSpiRamReadByte(0x58);
	printf("SPI RAM read back: %X\n", data);
	BermudaThreadSleep(500);
	return;
}
