/*
 *  BermudaOS - AVR PWM implementation.
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

//! \file src/arch/avr/pwm.c PWM backend functions.

#include <bermuda.h>

#include <lib/binary.h>
#include <dev/pwm.h>

#include <arch/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/pwm.h>

#if ((TIMERS & B100) != B100)
#warning AVR hardware timer 2 should be enabled to use PWM functionality.
#endif

/**
 * \brief Initialize the given PWM structure.
 * \param timer Backend timer to support the PWM.
 * \param freq Frequency to run on the timer.
 * \todo Implement a frequency calculator.
 */
PUBLIC void BermudaAvrPwmInit(PWM *pwm, TIMER *timer)
{
	if(timer == NULL) {
		BermudaAvrTimer2Init();
		timer = TIMER2;
	}
	
	pwm->timer = timer;
	pwm->freq = AVR_PWM_BASE_FRQ;
	BermudaAvrTimerSetTop(timer, (F_CPU/AVR_PWM_DEFAULT_PS)/AVR_PWM_BASE_FRQ);
	BermudaTimerSetPrescaler(timer, B11);
	BermudaAvrTimerSetISR(timer, OVERFLOW_ISR | OUTPUT_COMPAREA_ISR);
}

/**
 * \brief Initialize a PWM channel.
 * \param pwm PWM which contains an empty channel.
 * \param channel Channel to initialize.
 * \param bank I/O bank to generate the PWM signal on.
 * \param pin Pin number in the bank to toggle.
 * \see PWM_CHANNEL_NUM
 */
PUBLIC void BermudaAvrPwmChannelInit(PWM *pwm, PWM_CHANNEL_NUM channel, reg8_t bank,
									 unsigned char pin)
{
	PWM_CHANNEL *chan = BermudaHeapAlloc(sizeof(PWM_CHANNEL));
	pwm->channels[channel] = chan;
	
	chan->bank = bank;
	chan->pin = pin;
	chan->duty = 0;
	chan->flags = 0;
}

/**
 * \brief Set the duty cycle for a given PWM channel.
 * \param pwm PWM which contains <b>channel</b>.
 * \param duty Duty time to set in microseconds.
 * \param channel Channel to set the duty for.
 * \see PWM_CHANNEL_NUM
 */
PUBLIC void BermudaAvrPwmSetDuty(PWM *pwm, uint16_t duty, PWM_CHANNEL_NUM channel)
{
	PWM_CHANNEL *chan = pwm->channels[channel];
	
	chan->duty = duty;
	chan->flags |= PWM_CHANNEL_ENABLE;
}

