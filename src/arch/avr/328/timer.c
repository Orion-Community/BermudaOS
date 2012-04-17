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

#include <lib/binary.h>
#include <avr/interrupt.h>
#include <arch/avr/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/328/timer.h>

PRIVATE WEAK TIMER *timer0 = NULL;

/**
 * \fn BermudaInitTimer0()
 * \brief Initialize timer 0.
 * 
 * This function initializes timer 0 with the following properties:
 * 
 * * ISR type:                  Overflow
 * * Prescaler:                 64
 * * TOP:                       244
 * * Generated frequency:       1024Hz
 * 
 * The main function of this timer is to provide sleep support and generate a
 * timer feed to the scheduler.
 */
void BermudaInitTimer0()
{
        cli();
        timer0 = malloc(sizeof(*timer0));
        if(timer0 == NULL)
                return;
        
        timer0->tccr0a = &BermudaGetTCCR0A();
        timer0->tccr0b = &BermudaGetTCCR0B();
        
        /* mode: fast pwm, BOTTOM (0x0) to OCR0A */
        BermudaTimerSetWaveFormMode(timer0, B111);

        /* set prescaler */
        BermudaTimerSetPrescaler(timer0, B11);
        
        /* Set the top to 244 */
        BermudaGetOCR0A() = 244;
        
        /* Enable the overflow interrupt */
        spb(BermudaGetTIMSK0(), TOIE0);
}

#ifdef __LAZY__
/**
  * \fn BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres)
  * \brief Set the timer prescaler.
  * \param timer The prescaler will be set for this given timer.
  * \param pres The prescaler to set.
  * \return Error code.
  *
  * The prescaler must be a power of two, but there are a few exceptions. The
  * followeing values have a different meaning:
  *
  * * [pres == 0] This will disable the counter.
  * * [pres == 0xFF] This will set a prescaler of 1 (i.e. no prescaler)
  * * [pres == 0xFE] This will set the prescaler to an external clock on the T0
  *                  pin - falling edge.
  * * [pres == 0xFD] Same as <i>pres == 0xFE</i>, but it will clock on the rising
  *                  rising edge.
  */
int BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres)
{
        if(0xFF == pres || 0xFE == pres || 0xFD == pres)
                goto SetPrescaler;
        if(BermudaIsPowerOfTwo(pres))
                return -1;

        SetPrescaler:
        
        cpb(*timer->tccr0b, CS00);
        cpb(*timer->tccr0b, CS01);
        cpb(*timer->tccr0b, CS02);
        
        timer->prescaler = pres;
        
        switch(pres)
        {
                case 8:
                        spb(*timer->tccr0b, CS01);
                        break;
                        
                case 64:
                        spb(*timer->tccr0b, CS00);
                        spb(*timer->tccr0b, CS01);
                        break;
                        
                case 256:
                        spb(*timer->tccr0b, CS02);
                        break;
                        
                case 1024:
                        spb(*timer->tccr0b, CS00);
                        spb(*timer->tccr0b, CS02);
                        break;
                
                case 0xFD:
                        spb(*timer->tccr0b, CS00);
                        spb(*timer->tccr0b, CS01);
                        spb(*timer->tccr0b, CS02);
                        break;
                        
                case 0xFE:
                        spb(*timer->tccr0b, CS01);
                        spb(*timer->tccr0b, CS02);
                        break;
                        
                case 0xFF: /* a pres of 255 means there will not be a prescaler */
                        spb(*timer->tccr0b, CS00);
                        
                default:
                        break;
        }
        
        return 0;
}
#else
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
void BermudaTimerSetPrescaler(TIMER *timer, unsigned char pres)
{
        *timer->tccr0b &= ~B111; // results in 11111000
        *timer->tccr0b |= pres & B111;
}
#endif

#ifdef __LAZY__
/**
  * \fn BermudaTimerSetOutputCompareMatch(TIMER *timer, ocm_t ocm)
  * \brief Set the Output Compare Match for the given timer.
  * \param timer The timer.
  * \param ocm The output compare mode
  *
  * If bit 7 of <i>ocm</i> is set, the OCMB bits will be set instead of the
  * OCMA bits.
  */
PRIVATE WEAK void BermudaTimerSetOutputCompareMatch(TIMER *timer, ocm_t ocm)
{
        unsigned char bit0, bit1;
        
        if(NULL == timer)
                return;
        
        if((ocm & 0x80) != 0)
        {
                bit0 = COM0B0;
                bit1 = COM0B1;
                ocm &= ~0x80;
        }
        else
        {
                bit0 = COM0A0;
                bit1 = COM0A1;
        }
        
        cpb(*timer->tccr0a, bit0);
        cpb(*timer->tccr0a, bit1);
        
        switch(ocm)
        {
                case 1:
                        spb(*timer->tccr0a, bit0);
                        break;

                case 2:
                        spb(*timer->tccr0a, bit1);
                        break;

                case 3:
                        spb(*timer->tccr0a, bit0);
                        spb(*timer->tccr0a, bit1);
                        break;

                default:
                        break;
        }
        
        return;
}
#else
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
        *timer->tccr0a &= B11110000;
        *timer->tccr0a |= (ocm & B1111) << 4;
}
#endif

#ifdef __LAZY__
/**
 * \fn BermudaTimerSetWaveFormMode(TIMER *timer, wfm_t mode)
 * \brief Set the waveform generation mode
 * \param timer The timer.
 * \param mode The mode to set to <i>timer</i>.
 * \see enum wfm_t
 * 
 * This function will set the mode of the waveform generator of the given timer.
 * Valid modes are found in the enumeration wfm_t.
 */
PRIVATE WEAK void BermudaTimerSetWaveFormMode(TIMER *timer, wfm_t mode)
{
        if(NULL == timer)
                return;
        BermudaTimerDisable(timer);
        
        /* reset the entire operation mode */
        cpb(*timer->tccr0a, WGM00);
        cpb(*timer->tccr0a, WGM01);
        cpb(*timer->tccr0b, WGM02);
        
        switch(mode)
        {
                case WFM_PHASE_CORRECT:
                        spb(*timer->tccr0a, WGM00);
                        spb(*timer->tccr0b, WGM02);
                        break;
                
                case WFM_PHASE_CORRECT_MAX:
                        spb(*timer->tccr0a, WGM00);
                        break;
                
                case WFM_CTC:
                        spb(*timer->tccr0a, WGM01);
                        break;
                
                case WFM_FAST_PWM:
                        spb(*timer->tccr0a, WGM00);
                        spb(*timer->tccr0a, WGM01);
                        spb(*timer->tccr0b, WGM02);
                        break;
                
                case WFM_FAST_PWM_MAX:
                        spb(*timer->tccr0a, WGM00);
                        spb(*timer->tccr0a, WGM01);
                        break;
                        
                default: /* normal operation */
                        break;
        }
        
        BermudaTimerEnable(timer);
}
#else
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
        *timer->tccr0a &= ~B11; // clear WGM00 & WGM01
        *timer->tccr0b &= ~B1000; // clear WGM02
        
        *timer->tccr0a |= mode & B11;
        *timer->tccr0b |= (mode & B1) << 3;
        return;
}
#endif

static unsigned long timer_count = 0;
SIGNAL(TIMER0_OVF_vect)
{
        timer_count++;
}

inline unsigned long BermudaGetTimerCount()
{
        return timer_count;
}
