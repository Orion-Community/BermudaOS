/*
 *  BermudaOS - Mutex header
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

#ifndef __MUTEX_H_
#define __MUTEX_H_

#include <stdlib.h>

/**
 * \brief Mutex type definition.
 */
typedef struct mutex
{
	uint8_t lock; //!< Defines wether this mutex is locked or not.
	size_t interest; //!< Amount of threads showing interest in this mutex.
} mutex_t;

__DECL
extern void mutex_enter(mutex_t *mutex);
extern uint8_t mutex_leave(mutex_t *mutex);
extern size_t mutex_interest(mutex_t *mutex);
__DECL_END
#endif
