/*
 *  BermudaOS - DS3232 RTC driver
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

/* static vars */
//static struct i2c_client *rtc_client;

/* static functions */
//static int ds3232_resync(tm_t *time);
//static int ds3232_get(tm_t *time);
//static int ds3232_set(tm_t *time);

PUBLIC struct rtc *ds3232_init()
{
// 	struct rtc *rtc = rtc_init_clock(NULL, &ds3232_set, &ds3232_get, &ds3232_resync);
	return NULL;
}

/*
static int ds3232_resync(tm_t *time)
{
	return -1;
}

static int ds3232_get(tm_t *time)
{
	return -1;
}

static int ds3232_set(tm_t *time)
{
	return -1;
}
*/

