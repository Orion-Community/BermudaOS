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
  * \brief Set the prescaler of <i>timer</i> to <i>pres</i>.
  * \param timer The timer to set the prescaler for.
  * \param pres The prescaler to set.
  * \return Return code.
  *
  * This function sets the desired prescaler for a timer. The timer structure
  * has to be initialized and the prescaler has to be a power of two.
  */
int BermudaTimerSetPrescaler(TIMER *timer, unsigned short pres)
{
        if(0xFF == pres)
                goto SetPrescaler;
        if(BermudaIsPowerOfTwo(pres))
                return -1;

        SetPrescaler:
        
        cpb(*timer->tccr0b, CS00);
        cpb(*timer->tccr0b, CS01);
        cpb(*timer->tccr0b, CS02);

        switch(pres)
        {
                case 0: /* a prescaler of 0 disalbes the timer */
                        timer->prescaler = 0;
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
