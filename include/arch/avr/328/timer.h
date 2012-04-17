/*
 *  BermudaOS - Timers
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

#ifndef __TIMER328_H
#define __TIMER328_H

#include <bermuda.h>
#include <arch/avr/io.h>
#include <arch/avr/timer.h>

#define BermudaGetTCCR0A() SFR_IO8(0x24)
#define BermudaGetTCCR0B() SFR_IO8(0x25)

#define BermudaGetTCNT0() SFR_IO8(0x26)

#define BermudaGetOCR0A() SFR_IO8(0x27)
#define BermudaGetOCR0B() SFR_IO8(0x28)

#define BermudaGetTIMSK0() MEM_IO8(0x6E)
#define BermudaGetTIFR0()  SFR_IO8(0x15)

__DECL
extern void BermudaInitTimer0();
extern inline unsigned long BermudaGetTimerCount();

#ifdef __LAZY__
PRIVATE WEAK void BermudaTimerSetWaveFormMode(TIMER *timer, wfm_t mode);
PRIVATE WEAK void BermudaTimerSetOutputCompareMatch(TIMER *timer, ocm_t ocm);
#else
PRIVATE WEAK void BermudaTimerSetWaveFormMode(TIMER *timer, unsigned char mode);
PRIVATE WEAK void BermudaTimerSetOutputCompareMatch(TIMER *timer,
                                                        unsigned char ocm);
#endif

static inline void BermudaTimerDisable(TIMER *timer)
{
        unsigned short prescaler = timer->prescaler;
        BermudaTimerSetPrescaler(timer, DISABLE); /* disable the timer */
        timer->prescaler = prescaler;
        return;
}

static inline void BermudaTimerEnable(TIMER *timer)
{
        BermudaTimerSetPrescaler(timer, timer->prescaler);
        return;
}
__DECL_END

#endif /* __TIMER328_H */