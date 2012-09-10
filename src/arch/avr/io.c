/*
 *  BermudaOS - I/O
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

/** \file arch/avr/io.c */

#include <bermuda.h>
#include <lib/binary.h>

#include <arch/avr/io.h>

/**
 * \fn BermudaMutexEnter(unsigned char *lock)
 * \brief Enter locking state.
 * \param lock Lock pointer.
 * 
 * This function locks a variable mutually exclusive.
 */
void BermudaMutexEnter(volatile unsigned char *lock)
{
	while((*lock & B1) == B1);    
}

/**
 * \fn BermudaMutexRelease(unsigned char *lock)
 * \brief Release the mutex lock from <i>lock</i>.
 * \param lock Lock pointer.
 * 
 * This function releases the lock from <i>lock</i> mutually exclusive.
 */
inline void BermudaMutexRelease(volatile unsigned char *lock)
{
	BermudaEnterCritical();
	*lock = 0;
	BermudaExitCritical();
}
