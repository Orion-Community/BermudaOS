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
#ifndef __PWM_AVR_H
#define __PWM_AVR_H

__DECL
extern void BermudaAvrPwmInit(PWM *pwm, TIMER *timer, uint32_t freq);
extern void BermudaAvrPwmChannelInit(PWM *pwm, PWM_CHANNEL_NUM channel, reg8_t bank,
									 unsigned char pin);
extern void BermudaAvrPwmSetDuty(PWM *pwm, uint16_t duty, PWM_CHANNEL_NUM channel);
__DECL_END

#endif /* __PWM_AVR_H */
#endif /* PWM || DOXYGEN */