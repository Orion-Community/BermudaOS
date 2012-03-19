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
                _delay_ms(100);
                LED_PORT &= ~_BV(LED);
                _delay_ms(100);
        }
}

int main(void)
{
        BermudaInitUART();
        LED_DDR = 0xFF;
        spb(LED_PORT, LED);

        BermudaInitTimer0();
        BermudaInitBaseADC();
        sei();
        
        struct adc *adc = BermudaGetADC();
        BermudaSetPinMode(14, INPUT); /* pin A0 */
        while(1)
        {
                float raw_temp = adc->read(0) ;
                float temperature = raw_temp / 1024 * 500;
                printf("Analog pin 0 value: %f\n", temperature);
                flash_led(3);
                _delay_ms(1000);
        }
        return 0;
}
