/*
 *  BermudaOS - General purpose UART
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

#ifndef __ARCH_GENERIC_UART_H
#define __ARCH_GENERIC_UART_H

#include <stdio.h>
#include <bermuda.h>

#include <dev/usartif.h>

#define USART0 (&BermudaUART0)

extern int BermudaInitUART();
extern int BermudaUARTPutChar(char, FILE *);
extern int BermudaUARTGettChar(FILE *);
extern inline FILE *BermudaGetUARTOutput();
extern inline FILE *BermudaGetUARTInput();

struct hw_uart
{
	reg8_t ucsr0a; //!< UART control & status register 0-a.
	reg8_t ucsr0b; //!< UART control & status register 0-b.
	reg8_t ucsr0c; //!< UART control & status register 0-c.
	reg8_t ubrr0l; //!< UART bit rate register 0 LSB.
	reg8_t ubrr0h; //!< UART bit reate register 0 MSB.
	reg8_t udr0;   //!< UART data register 0.
} __attribute__((packed));

typedef struct hw_uart USART;

extern void BermudaUART0Init(USARTBUS *bus);

#endif /* __ARCH_GENERIC_UART_H */
