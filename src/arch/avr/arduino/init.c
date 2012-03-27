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
#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <arch/avr/timer.h>

#define LED_DDR  BermudaGetDDRB()
#define LED_PORT BermudaGetPORTB()
#define LED_PIN  BermudaGetPINB()
#define LED      PINB5

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
                LED_PORT |= _BV(LED);
                _delay_ms(1);
                LED_PORT &= ~_BV(LED);
                _delay_ms(1);
        }
}

void spi_init()
{
        DDRB = (1<<3)|(1<<5);
        PORTB |= 1<<2;
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
        printf("Master: %X\n", SPCR & (1<<MSTR));
}

void spi_write(unsigned char data)
{
        /* Start transmission */
        SPDR = data;
        /* Wait for transmission complete */
        while(!(SPSR & (1<<SPIF)))
        {
                printf("Waiting - Master: %X\n", SPCR & (1<<MSTR));
                _delay_ms(500);
        }
}

int main(void)
{
        BermudaInitUART();

//         BermudaInitTimer0();
//         BermudaInitBaseADC();
        sei();
        char buf = 'A';
        spi_init();
        spb(LED_PORT, LED);
//         BermudaSpiInit(malloc(sizeof(SPI)));
//         LED_DDR = (1<<3)|(1<<5);
        
//         struct adc *adc = BermudaGetADC();
//         BermudaSetPinMode(14, INPUT); /* pin A0 */
        while(1)
        {
                printf("Master: %X\n", SPCR & (1<<MSTR));
                spi_write(buf);
                _delay_ms(1000);
//                 float raw_temp = adc->read(0);
//                 int temperature = raw_temp / 1024 * 5000;
//                 temperature /= 10;
//                 printf("Temperature: %f - Free memory: %d\n", raw_temp,
//                         BermudaFreeMemory()
//                 );
//                 _delay_ms(1000);
        }
        return 0;
}
