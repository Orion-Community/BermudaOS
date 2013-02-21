/*
 *  BermudaOS - RTC library
 *  Copyright (C) 2012   Michel Megens <dev@michelmegens.net>
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

#ifndef __RTC_H_
#define __RTC_H_

#include <stdlib.h>
#include <stdio.h>

#include <lib/time.h>

typedef int (*set_time_hook_t)(tm_t *time);
typedef int (*get_time_hook_t)(tm_t *time);
typedef int (*resync_hook_t)(tm_t *time);

struct rtc
{
	int (*set_time)(tm_t *time); //!< Set time.
	int (*get_time)(tm_t *time); //!< Get time from hardware.
	int (*resync)(tm_t *time); //!< Resynch the clock.
	void *priv; //!< Priv data.
};

struct rtc *rtc_init_clock(void *priv, set_time_hook_t set, get_time_hook_t get,
								  resync_hook_t resync);

#endif
