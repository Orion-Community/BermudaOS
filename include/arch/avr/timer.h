/*
 *  BermudaOS - Timer
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

#ifndef __TIMER_H
#define __TIMER_H

#include <bermuda.h>

struct timer
{
        char *name;
        unsigned char id;
        
        unsigned long tick;
        
        /* timer config */
        unsigned char prescaler : 3;
        unsigned char mode : 3;
        unsigned short top;
        
        /* I/O registers */
        volatile unsigned char *tccr0a, *tccr0b, *tcnt0, *ocr0a, *ocr0b, 
                               *timsk0, *tifr0;
} __PACK__;
typedef struct timer TIMER;

typedef enum
{
        DISABLE,
        TOGGLE,
        CLEAR,
        SET,
} ocm_t;

__DECL
extern void BermudaSetupTimer(char *name, unsigned short top, unsigned char mode,
                                unsigned char prescaler);
extern int BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres);
__DECL_END

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
        #include <arch/avr/328/timer.h>
#endif

#endif /* __TIMER_H */
