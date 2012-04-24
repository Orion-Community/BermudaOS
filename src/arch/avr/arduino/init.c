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
#include <stdio.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#include <util/delay.h>

#include <sys/thread.h>
#include <sys/sched.h>

#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <arch/avr/timer.h>

#include <lib/spiram.h>
#include <lib/binary.h>

#define LED_DDR  BermudaGetDDRB()
#define LED_PORT BermudaGetPORTB()
#define LED_PIN  BermudaGetPINB()
#define LED      PINB5

#ifdef __THREAD_DBG__
static THREAD MainT;
static THREAD *th = NULL;
static THREAD *th2 = NULL;
#endif

extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;

/* Calculates the size of the free list */
int BermudaFreeListSize() {
        struct __freelist* current;
        int total = 0;

        for (current = __flp; current; current = current->nx) 
        {
                total += 2; /* Add two bytes for the memory block's header  */
                total += (int) current->sz;
        }

        return total;
}

int BermudaFreeMemory() 
{
        int free_memory;

        if ((int)__brkval == 0) 
        {
                free_memory = ((int)&free_memory) - ((int)&__heap_start);
        } 
        else 
        {
                free_memory = ((int)&free_memory) - ((int)__brkval);
                free_memory += BermudaFreeListSize();
        }
        return free_memory;
}

void flash_led(uint8_t count)
{
        while (count--)
        {
                LED_PORT |= BIT(LED);
                _delay_ms(100);
                LED_PORT &= ~BIT(LED);
                _delay_ms(100);
        }
}

#ifdef __THREAD_DBG__
// #ifndef THREADS
//         #error Thread debugging not possible without threads
// #endif
THREAD(TestThread, data)
{
        unsigned char led = 1;
//         char x = 0;
        while(1)
        {
                BermudaThreadEnterIO(BermudaCurrentThread);
                BermudaDigitalPinWrite(13, led);
                printf("thread 1\n");
                _delay_ms(100);
                BermudaThreadExitIO(BermudaCurrentThread);
                _delay_ms(1);
                led ^= 1;                
        }
}

THREAD(TestThread2, data)
{
        unsigned char led = 1;
//         char x = 0;    
        while(1)
        {
                BermudaThreadEnterIO(BermudaCurrentThread);
                BermudaDigitalPinWrite(13, led);
                printf("thread 2\n");
                _delay_ms(100);
                BermudaThreadExitIO(BermudaCurrentThread);
                _delay_ms(1);
                led ^= 1;
        }
}


THREAD(MainThread, data)
{
        unsigned char led = 1;
//         char x = 0;
        while(1)
        {
                BermudaThreadEnterIO(BermudaCurrentThread);
                BermudaDigitalPinWrite(13, led);
                printf("Main tread\n");
                _delay_ms(100);
                BermudaThreadExitIO(BermudaCurrentThread);
                _delay_ms(1);
                led ^= 1;
        }
}
#endif

void setup()
{
#ifdef __THREAD_DBG__
        BermudaSetPinMode(13, OUTPUT);
        th = malloc(sizeof(*th));
        th2 = malloc(sizeof(*th));
        BermudaThreadInit(th, "Test Thread", TestThread, NULL, 128, malloc(128),
                          BERMUDA_DEFAULT_PRIO);
        BermudaThreadInit(th2, "Lala Thread", TestThread2, NULL, 128, malloc(128),
                          BERMUDA_DEFAULT_PRIO);
        BermudaSchedulerInit(&MainT, &MainThread);
        BermudaSchedulerAddThread(th);
        BermudaSchedulerAddThread(th2);
        BermudaSchedulerStart();
#endif
#ifdef __SPIRAM__
        BermudaSpiRamWriteByte(0x58, 0x99);
#endif
}

int main(void)
{        
        BermudaInitUART();
        BermudaInitTimer0();
        BermudaInitBaseADC();
        
#ifdef __SPI__
        SPI *spi_if = malloc(sizeof(*spi_if));
        BermudaSpiInit(spi_if);
#ifdef __SPIRAM__
        BermudaSpiRamInit();
#endif
#endif
        sei();        
        setup();

        while(1)
        {
#ifdef __SPIRAM__
                unsigned int x = 0;

                _delay_ms(20);
                x = (unsigned int)BermudaSpiRamReadByte(0x58);
                _delay_ms(20);

                printf("Data byte readback: %x\n", x);
#endif
        }
        return 0;
}
