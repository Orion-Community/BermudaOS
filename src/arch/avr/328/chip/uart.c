/*
 *  BermudaOS - UART
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
#include <bermuda.h>
#include <arch/avr/io.h>

#include <lib/binary.h>
#include <sys/sched.h>

/* PRIVATE functions */
static void BermudaRedirUARTIO();

int BermudaInitUART()
{
        unsigned char ints = 0;
        BermudaSafeCli(&ints);
        
        UBRR0H = UBRRH_VALUE;
        UBRR0L = UBRRL_VALUE;
        
#ifdef UART2X
        UCSR0A |= _BV(U2X0);
#else
        UCSR0A &= ~(_BV(U2X0));
#endif
        
        UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* sent data in packets of 8 bits */
        UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Tx and Rx enable flags to 1 */
        
        BermudaRedirUARTIO();
        
        BermudaIntsRestore(ints);
        return 0;
}

int BermudaUARTPutChar(char c, FILE *stream)
{
#ifdef __THREADS__
        BermudaThreadEnterIO(BermudaCurrentThread);
#endif
        if(c == '\n')
                BermudaUARTPutChar('\r', stream);
        
        while((UCSR0A & _BV(UDRE0)) == 0);
        UDR0 = c;
#ifdef __THREADS__
        BermudaThreadExitIO(BermudaCurrentThread);
#endif
        return 0;
}

int BermudaUARTGetChar(FILE *stream)
{
        while((UCSR0A & RXC0) == 0);
        return UDR0;
}

static FILE uartout = {0};
static FILE uartinput = {0};
static void BermudaRedirUARTIO()
{
        fdev_setup_stream(&uartout, BermudaUARTPutChar, NULL, _FDEV_SETUP_WRITE);
        fdev_setup_stream(&uartinput, NULL, BermudaUARTGetChar, _FDEV_SETUP_READ);
        stdout = &uartout;
        stdin  = &uartinput;
}

inline FILE *BermudaGetUARTOutput()
{
        return &uartout;
}

inline FILE *BermudaGetUARTInput()
{
        return &uartinput;
}
