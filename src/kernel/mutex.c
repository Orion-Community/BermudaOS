/*
 *  BermudaOS - Mutexes.
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

#include <stdlib.h>
#include <stdint.h>

#include <kernel/mutex.h>

/**
 * \brief Return the amount of threads which are interested in this mutex.
 * \param mutex Mutex to check the interest of.
 * \return Amount of threads which are showing interest.
 * \retval 0 No threads are showing interest or \p mutex is \p NULL.
 */
PUBLIC size_t mutex_interest(mutex_t *mutex)
{
	if(!mutex) {
		return 0;
	}
	return mutex->interest;
}

/**
 * \brief Lock a mutex.
 * \param mutex Mutex to lock.
 *
 * The given mutex \p mutex will be locked. If it is already locked it will wait for an unlock in
 * a busy waiting loop.
 */
PUBLIC void mutex_enter(mutex_t *mutex)
{
	uint8_t tmp = 1;

	while(tmp) {
		mutex->lock ^= tmp;
		tmp ^= mutex->lock;
		mutex->lock ^= tmp;
	}
}

/**
 * \brief Unlock a mutex.
 * \param mutex Mutex to unlock.
 * \return The state of the mutex before the unlock.
 * \retval 0 The mutex was already unlocked.
 * \retval 1 The mutex was locked before calling this function.
 */
PUBLIC uint8_t mutex_leave(mutex_t *mutex)
{
	uint8_t ret = mutex->lock;
	mutex->lock = 0;
	return ret;
}

