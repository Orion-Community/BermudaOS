/*
 *  BermudaOS - Time definitions
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

#ifndef __TIME_H_
#define __TIME_H_

#include <stdlib.h>
#include <stdio.h>

typedef struct time
{
	uint8_t sec; //!< Seconds after the minute.
	uint8_t min; //!< Minutes after the hour.
	uint8_t hour; //!< Hours since midnight.
	uint8_t mday; //!< Day of the month.
	uint8_t month; //!< Month since January.
	int32_t year; //!< Year since 1970.
	bool dst; //!< 1 when DST is active, 0 if not.
} tm_t;

#endif