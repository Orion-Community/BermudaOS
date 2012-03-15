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

#include <arch/avr/io.h>
#include <avr/io.h>

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

inline unsigned short BermudaReadPGMWord(unsigned short addr)
{
        unsigned short result;
        __asm__ __volatile__("\n\t"
                             "lpm %A0, Z+\n\t"
                             "lpm %B0, Z"
                             : "=r" (result)
                             : "z" (addr)
        );
        return result;
}
