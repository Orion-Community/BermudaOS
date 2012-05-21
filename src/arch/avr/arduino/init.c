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

#include <stdlib.h>

#include <avr/interrupt.h>

#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/virt_timer.h>

#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/stack.h>

#include <lib/spiram.h>
#include <lib/binary.h>

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
        while(1)
        {
                loop();
        }
}
#endif

PUBLIC int BermudaInit(void)
{       
        BermudaHeapInitBlock((volatile void*)&__heap_start, MEM-64);
        BermudaInitUART();
        BermudaInitTimer0();

#ifdef __ADC__
        BermudaInitBaseADC();
#endif
        
#ifdef __SPI__
        SPI *spi_if = BermudaHeapAlloc(sizeof(*spi_if));
        BermudaSpiHardwareInit(spi_if);  
#endif
        
        STACK_L = (MEM-256) & 0xFF;
        STACK_H = ((MEM-256) >> 8) & 0xFF;
        sei();
        BermudaTimerInit();
        
#ifdef __THREADS__
        BermudaSchedulerInit(&MainThread);
        BermudaSchedulerStart();
#else
        while(1)
        {
                loop();
        }
#endif
        
        return 0;
}
