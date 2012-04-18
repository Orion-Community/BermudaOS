/*
 *  BermudaOS - Stack support :: Used by the general schedule module
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

#include <bermuda.h>
#include <sys/thread.h>

#include <arch/avr/stack.h>
#include <arch/avr/io.h>

void BermudaStackInit(stack_t sp, unsigned short stack_size, thread_handle_t handle)
{
        /*
         * first we add the function pointer to the stack
         */
        *(sp--) = (unsigned short)handle & 0xff;
        *(sp--) = ((unsigned short)handle >> 8) & 0xff;
        
        /* add the SREG register */
        *(sp--) = 0x0; // location of R0 normally
        *(sp--) = *AvrIO->sreg;
        
        /* pad the other registers */
        int i = 0;
        for(; i < 31; i++)
        	sp[i] = 0;
}
