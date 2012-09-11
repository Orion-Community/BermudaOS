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

#if defined(__PWM__) || defined(__DOXYGEN__)

#include <bermuda.h>

#include <lib/binary.h>
#include <dev/pwmdev.h>

#include <arch/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/pwm.h>

PUBLIC void BermudaAvrPwmInit(PWM *pwm, TIMER *timer, uint32_t freq)
{
	pwm->timer = timer;
	pwm->freq = freq;
	
	BermudaTimerSetPrescaler(timer, B001);
}

#endif