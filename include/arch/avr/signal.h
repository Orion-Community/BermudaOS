/*
 *  HomeWeather system - External signals
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

#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <avr/interrupt.h>

/* signal definitions */
#ifdef SIGNAL
#undef SIGNAL
#endif

#ifdef __cplusplus
#define SIGNAL(vec) \
static extern "C" void vec (void); \
static extern "C" void vec (void)
#else
#define SIGNAL(vec) \
static void vec (void); \
static void vec (void)
#endif

#ifdef _VECTOR
#define INT_RESET       _VECTOR(0)   /* interrupt reset */
#define INT0            _VECTOR(1)   /* External interrupt 0 */
#define INT1            _VECTOR(2)   /* External interrupt 1 */
#define PCINT0          _VECTOR(3)   /* Pin Change Interrupt Request 0 */
#define PCINT1          _VECTOR(4)   /* Pin Change Interrupt Request 0 */
#define PCINT2          _VECTOR(5)   /* Pin Change Interrupt Request 1 */
#define WDT             _VECTOR(6)   /* Watchdog Time-out Interrupt */
#define TIMER2_COMPA    _VECTOR(7)   /* Timer/Counter2 Compare Match A */
#define TIMER2_COMPB    _VECTOR(8)   /* Timer/Counter2 Compare Match A */
#define TIMER2_OVF      _VECTOR(9)   /* Timer/Counter2 Overflow */
#define TIMER1_CAPT     _VECTOR(10)  /* Timer/Counter1 Capture Event */
#define TIMER1_COMPA    _VECTOR(11)  /* Timer/Counter1 Compare Match A */
#define TIMER1_COMPB    _VECTOR(12)  /* Timer/Counter1 Compare Match B */
#define TIMER1_OVF      _VECTOR(13)  /* Timer/Counter1 Overflow */
#define TIMER0_COMPA    _VECTOR(14)  /* TimerCounter0 Compare Match A */
#define TIMER0_COMPB    _VECTOR(15)  /* TimerCounter0 Compare Match B */
#define TIMER0_OVF      _VECTOR(16)  /* Timer/Couner0 Overflow */
#define SPI_STC         _VECTOR(17)  /* SPI Serial Transfer Complete */
#define USART_RX        _VECTOR(18)  /* USART Rx Complete */
#define USART_UDRE      _VECTOR(19)  /* USART, Data Register Empty */
#define USART_TX        _VECTOR(20)  /* USART Tx Complete */
#define ADC_READY       _VECTOR(21)  /* ADC Conversion Complete */
#define EE_READY        _VECTOR(22)  /* EEPROM Ready */
#define ANALOG_COMP     _VECTOR(23)  /* Analog Comparator */
#define TWI             _VECTOR(24)  /* Two-wire Serial Interface */
#define SPM_READY       _VECTOR(25)  /* Store Program Memory Read */
#else
#error _VECTOR is not defined
#endif

#endif
