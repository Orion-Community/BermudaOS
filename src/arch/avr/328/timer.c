/*
 *  BermudaOS - Timer handlers
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

/** \file src/arch/avr/328/timer.c */
#include <lib/binary.h>

#include <arch/avr/interrupts.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/virt_timer.h>

#include <arch/avr/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/328/timer.h>

PRIVATE WEAK TIMER *timer0 = NULL;
PRIVATE WEAK TIMER *timer2 = NULL;

/**
 * \fn BermudaInitTimer0()
 * \brief Initialize timer 0.
 * 
 * This function initializes timer 0 with the following properties: \n
 * \n
 * * ISR type:                  Overflow \n
 * * Prescaler:                 64 \n
 * * TOP:                       250 \n
 * * Generated frequency:       1000Hz \n
 * \n
 * The main function of this timer is to provide sleep support and generate a
 * timer feed to the scheduler.
 */
void BermudaInitTimer0()
{
        cli();
        timer0 = BermudaHeapAlloc(sizeof(*timer0));
        if(timer0 == NULL)
                return;

        // fast pwm, TOP = OCR0A, prescaler 64, com disconnected
        BermudaTimer0InitRegs(timer0);
        BermudaHardwareTimerInit(timer0, B111, B11, B0);
        
        /* Set the top to 244 */
        *(timer0->output_comp_a) = 250;
        
        /* Enable the overflow interrupt */
        spb(*(timer0->int_mask), TOIE0);
}

#if (TIMERS & B100) == B100
/**
 * \fn BermudaInitTimer2()
 * \brief Initialize timer 2.
 *
 * This function initializes timer 2 with the following properties:
 *
 * * ISR type:                  Overflow
 * * Prescaler:                 32
 * * TOP:                       250
 * * Generated frequency:       2000Hz
 *
 * The main function of this timer is to provide sleep support and generate a
 * timer feed to the scheduler.
 */
void BermudaInitTimer2()
{
        unsigned char ints = 0;
        BermudaSafeCli(&ints);

        timer2 = BermudaHeapAlloc(sizeof(*timer2));
        BermudaTimer2InitRegs(timer2);
        BermudaHardwareTimerInit(timer2, B111, B11, B0);

        *(timer2->output_comp_a) = 250;
        spb(*timer2->int_mask, TOIE2);
        
        BermudaIntsRestore(ints);
}
#endif

void BermudaHardwareTimerInit(timer, waveform, prescaler, ocm)
unsigned char waveform, prescaler, ocm;
TIMER *timer;
{
        if(NULL == timer)
                return;
        
        BermudaTimerSetOutputCompareMatch(timer, ocm);
        BermudaTimerSetPrescaler(timer, prescaler);
        BermudaTimerSetWaveFormMode(timer, waveform);
}

#if (TIMERS & B1) == B1
PRIVATE WEAK void BermudaTimer0InitRegs(TIMER *timer)
{
        timer->controlA      = &BermudaGetTCCR0A();
        timer->controlB      = &BermudaGetTCCR0B();
        timer->int_mask      = &BermudaGetTIMSK0();
        timer->int_flag      = &BermudaGetTIFR0();
        timer->output_comp_a = &BermudaGetOCR0A();
        timer->output_comp_b = &BermudaGetOCR0B();
        timer->countReg      = &BermudaGetTCNT0();
        return;
}
#endif

#if (TIMERS & B10) == B10
#error No support for timer 1 yet!
PRIVATE WEAK void BermudaTimer1InitRegs(TIMER *timer);
#endif

#if (TIMERS & B100) == B100
// #error No support for timer 2 yet!
PRIVATE WEAK void BermudaTimer2InitRegs(TIMER *timer)
{
        timer->controlA      = &BermudaGetTCCR2A();
        timer->controlB      = &BermudaGetTCCR2B();
        timer->int_mask      = &BermudaGetTIMSK2();
        timer->int_flag      = &BermudaGetTIFR2();
        timer->output_comp_a = &BermudaGetOCR2A();
        timer->output_comp_b = &BermudaGetOCR2B();
        timer->countReg      = &BermudaGetTCNT2();
}
#endif

#if (TIMERS & B100) == B100
// #error No support for timer 2 yet!
PRIVATE WEAK void BermudaTimerSetAsychStatusRegister(TIMER *timer,
                                                     unsigned char sr)
{
        TIMER2_ASYC_SR &= B10000011;
        TIMER2_ASYC_SR |= sr;
}
#endif

/**
  * \fn BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres)
  * \brief Set the timer prescaler.
  * \param timer The prescaler will be set for this given timer.
  * \param pres The prescaler to set.
  * \return Error code.
  * 
  * pres[0]: Represents CS00;
  * pres[1]: Represents CS01;
  * pres[2]: Represents CS02;
  */
PUBLIC void BermudaTimerSetPrescaler(TIMER *timer, unsigned char pres)
{
        *timer->controlB &= ~B111; // results in 11111000
        *timer->controlB |= pres & B111;
        timer->prescaler = pres & B111;
}

/**
  * \fn BermudaTimerSetOutputCompareMatch(TIMER *timer, ocm_t ocm)
  * \brief Set the Output Compare Match for the given timer.
  * \param timer The timer.
  * \param ocm The output compare mode
  *
  * ocm[0] COM0B0
  * ocm[1] COM0B1
  * ocm[2] COM0A0
  * ocm[3] COM0A1
  */
PRIVATE WEAK void BermudaTimerSetOutputCompareMatch(timer, ocm)
TIMER *timer;
unsigned char ocm;
{
        *timer->controlA &= B11110000;
        *timer->controlA |= (ocm & B1111) << 4;
}

/**
 * \fn BermudaTimerSetWaveFormMode(TIMER *timer, wfm_t mode)
 * \brief Set the waveform generation mode
 * \param timer The timer.
 * \param mode The mode to set to <i>timer</i>.
 *
 * mode[0]: Represents WGM00;
 * mode[1]: Represents WGM01;
 * mode[2]: Represents WGM02;
 */
PRIVATE WEAK void BermudaTimerSetWaveFormMode(TIMER *timer, unsigned char mode)
{
        *timer->controlA &= ~B11; // clear WGM00 & WGM01
        *timer->controlB &= ~B1000; // clear WGM02
        
        *timer->controlA |= mode & B11;
        *timer->controlB |= (mode & B1) << 3;
        return;
}

/**
 * \var BermudaSystemTick
 * \brief System ticks.
 * \see BermudaTimerGetSysTick
 * 
 * Amount of system ticks.
 */
static unsigned long BermudaSystemTick = 0;

/**
 * \brief Return the amount of system ticks.
 * \return Amount of system ticks.
 * \see BermudaSystemTick
 * \note This function will safely get the value of system ticks by disabling and
 *       re-enabling the interrupts.
 */
PUBLIC inline unsigned long BermudaTimerGetSysTick()
{
        unsigned long ret;
        
        BermudaEnterCritical();
        ret = BermudaSystemTick;
        BermudaExitCritical();
        return ret;
}

SIGNAL(TIMER0_OVF_vect)
{        
	BermudaSystemTick++;
}

#if (TIMERS & B100) == B100
SIGNAL(TIMER2_OVF_vect)
{

}
#endif
