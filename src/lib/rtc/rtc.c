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

#include <stdlib.h>
#include <stdio.h>

#include <lib/time.h>
#include <lib/rtc.h>

/**
 * \brief Initialize an RTC data structure.
 * \param priv Private chip data.
 * \param set Function hook to set the time.
 * \param get Function hook to get the time.
 * \param resync Function hook to resynchronize the time.
 */
PUBLIC struct rtc *rtc_init_clock(void *priv, set_time_hook_t set, get_time_hook_t get,
								  resync_hook_t resync)
{
	struct rtc *rtc = malloc(sizeof(*rtc));
	if(rtc) {
		rtc->priv = priv;
		rtc->resync = resync;
		rtc->set_time = set;
		rtc->get_time = get;
		return rtc;
	} else {
		return NULL;
	}
}