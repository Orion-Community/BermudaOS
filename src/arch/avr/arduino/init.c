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
#include <avr/io.h>

#include <util/delay.h>

#include <arch/avr/chip/uart.h>

#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_PIN  PINB
#define LED      PINB5

void flash_led(uint8_t count)
{
        while (count--) {
                LED_PORT |= _BV(LED);
                _delay_ms(100);
                LED_PORT &= ~_BV(LED);
                _delay_ms(100);
        }
}

void println(char *s)
{
        while(*s)
        {
                if(*s == '\n')
                {
                        BermudaUARTPutChar('\r', NULL);
                        _delay_ms(1);
                }
                BermudaUARTPutChar(*s, NULL);
                _delay_ms(1);
                s++;
        }
}

int main(void)
{
        BermudaInitUART();
        char x[128];
        LED_DDR = 0x0;
        unsigned char y = UCSR0A;
        sprintf(x, "Dit is de meester test: %x\n", y);
        printf(x);
        while(1)
        {
//                 flash_led(3);
        }
        return 0;
}
