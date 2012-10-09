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
#include <dev/adc.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/busses/atmega.h>

#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/virt_timer.h>

#include <arch/adc.h>
#include <arch/avr/interrupts.h>
#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <arch/avr/328/dev/spibus.h>
#include <arch/avr/timer.h>
#include <arch/avr/stack.h>
#include <arch/avr/serialio.h>
#include <arch/avr/328/dev/usart.h>

#define LED_DDR  BermudaGetDDRB()
#define LED_PORT BermudaGetPORTB()
#define LED_PIN  BermudaGetPINB()
#define LED      PINB5

extern unsigned int __heap_start;
#ifdef __THREADS__
extern void loop();
#else
extern unsigned long loop();
#endif
extern void setup();

#ifdef __TWI__
struct i2c_adapter i2c_adapter;
#endif

#ifdef __THREADS__
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
	BermudaHeapInitBlock((volatile void*)&__heap_start, (EXTRAM+MEM-128) - (size_t)&__heap_start);
#ifdef __USART__
	vfs_init();
	BermudaUsart0Init(USART0);
	BermudaUsartSetupStreams();
#endif
	BermudaInitTimer0();

#ifdef __ADC__
	BermudaAdc0Init();
#endif
#ifdef __SPI__
	BermudaSPI0HardwareInit();
#endif

#ifdef __TWI__
	atmega_i2c_c0_hw_init(ATMEGA_I2C_C0_SLA, &i2c_adapter);
#endif

	STACK_L = (MEM-128) & 0xFF;
	STACK_H = ((MEM-128) >> 8) & 0xFF;
	sei();
	BermudaTimerInit();

#ifdef __THREADS__
	BermudaSchedulerInit(&MainThread);
	BermudaSchedulerStart();
#else

	unsigned long current_ms, delay_ms = 0, prev_ms = 0, delay;
	
	setup();
	delay = loop(); // first run to get delay
	while(1) {
		current_ms = BermudaTimerGetSysTick();
		if(prev_ms != current_ms) {
			BermudaTimerProcess();
			prev_ms = current_ms;
		}

		if((current_ms - delay_ms) > delay) {
			delay = loop();
			delay_ms = current_ms;
		}
		
	}
#endif
        return 0;
}
