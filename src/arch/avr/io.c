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

/** \file io.c */

#include <arch/avr/io.h>
#include <avr/io.h>

/**
 * \brief Read a byte from program memory.
 * \param addr Addess to read from.
 * \return Data read from PGM.
 * 
 * Read data stored at <b>addr</b> in program memory.
 */
inline unsigned char BermudaReadPGMByte(unsigned short addr)
{
        unsigned char result;
        __asm__ __volatile__("\n\t"
                             "lpm %0, z\n\t"
                             : "=r" (result)
                             : "z"  (addr)
                     );
        return result;
}

/**
 * \brief Read a word from program memory.
 * \param addr Addess to read from.
 * \return Data read from PGM.
 * 
 * Read data stored at <b>addr</b> and <b>addr+1</b> in program memory.
 */
inline unsigned short BermudaReadPGMWord(unsigned short addr)
{
	unsigned short ret = BermudaReadPGMByte(addr);
	ret |= (BermudaReadPGMByte(addr+1) << 8);
	return ret;
}

/**
 * \fn BermudaMutexEnter(unsigned char *lock)
 * \brief Enter locking state.
 * \param lock Lock pointer.
 * 
 * This function locks a variable mutually exclusive.
 */
void BermudaMutexEnter(volatile unsigned char *lock)
{
        unsigned char test = 0x1;
        
        while(test != 0x0)
        {
                test  ^= *lock;
                *lock ^= test;
                test  ^= *lock;
        }
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
        unsigned char atomic = 0;
        
        atomic ^= *lock;
        *lock  ^= atomic;
        atomic ^= *lock;
}
