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
        unsigned short prescaler;
        
        /* I/O registers */
        volatile unsigned char *controlA, *controlB, *countReg, *output_comp_a,
                                *output_comp_b, *int_mask, *int_flag;
} __PACK__;
typedef struct timer TIMER;

typedef enum
{
        DISABLE,
        TOGGLE,
        CLEAR,
        SET,
} ocm_t;

typedef enum
{
        WFM_NORMAL,
        WFM_PHASE_CORRECT,
        WFM_PHASE_CORRECT_MAX,
        WFM_CTC,
        WFM_FAST_PWM_MAX,
        WFM_FAST_PWM,
} wfm_t;

__DECL
extern void BermudaSetupTimer(char *name, unsigned short top, unsigned char mode,
                                unsigned char prescaler);

#ifdef __LAZY__
extern int BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres);
#else
extern void BermudaTimerSetPrescaler(TIMER *timer, unsigned char pres);
#endif

extern inline unsigned long BermudaTimerGetSysTick();
__DECL_END

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
        #include <arch/avr/328/timer.h>
#endif

#endif /* __TIMER_H */
