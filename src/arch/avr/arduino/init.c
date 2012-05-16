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
#include <util/delay.h>

#include <sys/thread.h>
#include <sys/sched.h>

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

#ifdef __THREAD_DBG__
static THREAD *th = NULL;
static THREAD *th2 = NULL;
#endif

extern unsigned int __heap_start;

THREAD(TestThread, data)
{
#ifdef __SPIRAM__
        unsigned int rd = 0;
#else
        unsigned char led = 1;
#endif

        while(1)
        {

#ifdef __SPIRAM__
                rd = (unsigned int)BermudaSpiRamReadByte(0x58);
#ifdef __VERBAL__
                printf("Data byte readback: %x\n", rd);
#endif
#else
                BermudaDigitalPinWrite(12, led);
                led ^= 1;
                _delay_ms(1000);
#endif
                
                
        }
}

THREAD(TestThread2, data)
{
#ifdef __ADC__
        struct adc *adc = BermudaGetADC();
        int temperature = 0;
        float raw_temp = 0;
#endif
   
        while(1)
        {
#ifdef __ADC__
                raw_temp = adc->read(A0);
                temperature = raw_temp / 1024 * 5000;
                temperature /= 10;
#ifdef __VERBAL__
                printf("Temperature: %i - Free memory: %i\n", temperature,
                        BermudaHeapAvailable()
                );
#endif
#endif


                BermudaThreadSleep(1000);
        }
}


THREAD(MainThread, data)
{
        unsigned char led = 1;

        while(1)
        {
                BermudaDigitalPinWrite(13, led);
//                 printf("Available mem: %u - SREG %X\n", BermudaHeapAvailable(),
//                        *AvrIO->sreg);
                led ^= 1;
                _delay_ms(200);
                BermudaThreadSetPrio(200);
        }
}

PRIVATE WEAK void setup()
{
#ifdef __THREAD_DBG__
        th = BermudaHeapAlloc(sizeof(*th));
        th2 = BermudaHeapAlloc(sizeof(*th));
        BermudaSchedulerInit(&MainT, &MainThread);
        BermudaThreadCreate(th, "Test Thread", TestThread, NULL, 128, 
                          BermudaHeapAlloc(128), BERMUDA_DEFAULT_PRIO);
        BermudaThreadCreate(th2, "Test Thread 2", TestThread2, NULL, 128, 
                          BermudaHeapAlloc(128), BERMUDA_DEFAULT_PRIO);
#endif
#ifdef __SPIRAM__
        BermudaSpiRamWriteByte(0x58, 0x99);
#else
        BermudaSetPinMode(13, OUTPUT);
        BermudaSetPinMode(12, OUTPUT);
        BermudaSetPinMode(A0, INPUT);
#endif
}

int main(void)
{       
        BermudaHeapInitBlock((volatile void*)&__heap_start, MEM-64);
        BermudaInitUART();
        BermudaInitTimer0();
#if (TIMERS & B100) == B100
        BermudaInitTimer2();
#endif

#ifdef __ADC__
        BermudaInitBaseADC();
#endif
        
#ifdef __SPI__
        SPI *spi_if = BermudaHeapAlloc(sizeof(*spi_if));
        BermudaSpiHardwareInit(spi_if);
#ifdef __SPIRAM__
        BermudaSpiRamInit();
#endif
#endif
        STACK_L = (MEM-256) & 0xFF;
        STACK_H = ((MEM-256) >> 8) & 0xFF;
        sei();
        setup();
        
        THREAD *th = BermudaHeapAlloc(sizeof(*th));
//         THREAD *th2 = BermudaHeapAlloc(sizeof(*th2));
        
        BermudaSchedulerInit(&MainThread);
        BermudaThreadCreate(th, "Test Thread", TestThread, NULL, 128, 
                          BermudaHeapAlloc(128), 155);
//         BermudaThreadCreate(th2, "Test Thread 2", TestThread2, NULL, 128, 
//                           BermudaHeapAlloc(128), 155);
        
        BermudaSchedulerExec();
        while(1)
        {}
        return 0;
}
