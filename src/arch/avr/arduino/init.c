/*
 *  BermudaOS - External signals
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

#include <lib/spiram.h>
#include <lib/binary.h>
#include <dev/dev.h>

#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/virt_timer.h>

#include <arch/avr/interrupts.h>
#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <arch/avr/328/dev/spibus.h>
#include <arch/avr/timer.h>
#include <arch/avr/stack.h>
#include <arch/avr/328/dev/twibus.h>

#define LED_DDR  BermudaGetDDRB()
#define LED_PORT BermudaGetPORTB()
#define LED_PIN  BermudaGetPINB()
#define LED      PINB5

extern unsigned int __heap_start;

#ifdef __THREADS__

extern void loop();
extern void setup();

THREAD(MainThread, data)
{
	setup();
	while(1) {
		loop();
	}
}
#endif

PUBLIC int BermudaInit(void)
{       
	BermudaHeapInitBlock((volatile void*)&__heap_start, MEM-128);
	BermudaInitUART();
	BermudaInitTimer0();

#ifdef __ADC__
	BermudaInitBaseADC();
#endif
#ifdef __SPI__
	BermudaSPI0HardwareInit();
#endif

	BermudaTwi0Init(0x56);

	STACK_L = (MEM-128) & 0xFF;
	STACK_H = ((MEM-128) >> 8) & 0xFF;
	sei();
	BermudaTimerInit();

#ifdef __THREADS__
	BermudaSchedulerInit(&MainThread);
	BermudaSchedulerStart();
#else
	while(1) {
		loop();
	}
#endif
        return 0;
}
