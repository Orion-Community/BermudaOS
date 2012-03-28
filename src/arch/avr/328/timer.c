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

#include <avr/io.h>
#include <lib/binary.h>
#include <avr/interrupt.h>
#include <arch/avr/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/328/timer.h>

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
        
        /* mode: fast pwm, BOTTOM (0x0) to OCR0A */
        spb(BermudaGetTCCR0A(), WGM00);
        spb(BermudaGetTCCR0A(), WGM01);
        spb(BermudaGetTCCR0B(), WGM02);

        /* set prescaler */
        spb(BermudaGetTCCR0B(), CS00);
        spb(BermudaGetTCCR0B(), CS01);
        
        /* Set the top to 244 */
        BermudaGetOCR0A() = 244;
        
        /* Enable the overflow interrupt */
        spb(BermudaGetTIMSK0(), TOIE0);
        
        /* re-enable interrupts */
        sei();
}

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
                case 0:
                        break;
                        
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

PRIVATE WEAK void BermudaSetOutputCompareMatch(TIMER *timer, ocm_t ocm)
{
        if(NULL == timer)
                return;
        
        cpb(*timer->tccr0a, COM0A0);
        cpb(*timer->tccr0a, COM0A1);
        
        switch(ocm)
        {
                case 1:
                        spb(*timer->tccr0a, COM0A0);
                        break;

                case 2:
                        spb(*timer->tccr0a, COM0A1);
                        break;

                case 3:
                        spb(*timer->tccr0a, COM0A0);
                        spb(*timer->tccr0a, COM0A1);
                        break;

                default:
                        break;
        }
        
        return;
}

static unsigned long timer_count = 0;
SIGNAL(TIMER0_OVF_vect)
{
        timer_count++;
}

inline unsigned long BermudaGetTimerCount()
{
        return timer_count;
}
