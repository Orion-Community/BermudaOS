/*
 *  BermudaOS - Generic PWM header
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

/**
 * \file include/dev/pwmdev.h PWM device header.
 * \brief General PWM definitions.
 * 
 * All PWM's have a backend timer, which is defined in the TIMER structure. This
 * backend functionality is implemented in the correct architecture.
 */

#ifndef __PWMDEV_H
#define __PWMDEV_H

#include <bermuda.h>

#include <arch/io.h>

/**
 * \def MAX_CHANNELS
 * \brief Maximum ammount of channels per PWM.
 */
#define MAX_CHANNELS 4

/**
 * \def PWM_CHANNEL_ENABLE
 * \brief Channel enable flag.
 */
#define PWM_CHANNEL_ENABLE 1

/**
 * \def PWM_CHANNEL_DISABLE
 * \brief Channel disable flag.
 */
#define PWM_CHANNEL_DISABLE 0

/**
 * \brief Type definition of the PWM structure.
 * \see pwm
 */
typedef struct pwm PWM;

/**
 * \brief Type definition of the PWM channel.
 * \see pwm_channel
 */
typedef struct pwm_channel PWM_CHANNEL;

/**
 * \brief Type definition of the different PWM channels.
 */
typedef enum
{
	CHANNEL_ONE = 0, //!< First channel.
	CHANNEL_TWO,     //!< Second channel.
	CHANNEL_THREE,   //!< Third channel.
	CHANNEL_FOUR,    //!< Fourth channel.
} PWM_CHANNEL_NUM;

/**
 * \brief Definition of the PWM channel.
 * 
 * A PWM channel is the actual PWM which can generate a signal. Each PWM structure
 * can contain 4 channels.
 */
struct pwm_channel
{
	/**
	 * \brief I/O port bank.
	 * \note Banks are implemented in the architecture.
	 */
	reg8_t bank;
	unsigned char pin : 4; //! Pin number in te bank.
	
	/**
	 * \brief Flag member.
	 * 
	 * * BIT[0]: 1 = enabled, 0 = disabled.
	 */
	unsigned char flags : 1;
	
	uint16_t duty; //!< Duty time in ms.
} __attribute__((packed));

/**
 * \brief Definition of the PWM structure.
 * \see PWM
 */
struct pwm
{
	TIMER *timer; //!< Timer backend of this PWM.
	uint32_t freq; //!< PWM frequency in Hz.
	PWM_CHANNEL *channels[MAX_CHANNELS]; //! PWM channels
} __attribute__((packed));

#endif /* __PWMDEV_H */
