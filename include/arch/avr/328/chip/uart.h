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

#ifndef __UART_H
#define __UART_H

#include <stdlib.h>
#include <stdio.h>
#include <lib/binary.h>
#include <arch/avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BAUD
#define BAUD 9600
#warning "Serial baudrate has not been defined, defaulting to 9600."
#endif

#define UBRR0L MEM_IO8(0xC4)
#define UBRR0H MEM_IO8(0xC5)
#define UDR0 MEM_IO8(0xC6)

#define UCSR0A MEM_IO8(0xC0)
#define UCSR0B MEM_IO8(0xC1)
#define UCSR0C MEM_IO8(0xC2)

#define UBRRL_VALUE (BAUD & 0xFF)
#define UBRRH_VALUE ((BAUD >> 8) & 0xF)
#define U2X0 1
#define UCSZ00 1
#define UCSZ01 2
#define TXEN0 3
#define RXEN0 4
#define UDRE0 5
#define RXC0 7


extern int BermudaInitUART();
extern int BermudaUARTPutChar(char, FILE *);
extern int BermudaUARTGettChar(FILE *);
extern inline FILE *BermudaGetUARTOutput();
extern inline FILE *BermudaGetUARTInput();

#ifdef __cplusplus
}
#endif
#endif /* __UART_H */