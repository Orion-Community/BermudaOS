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

#if defined(__THREADS__) || defined(__DOXYGEN__)
#include <bermuda.h>
#include <sys/thread.h>

#include <arch/avr/stack.h>
#include <arch/avr/io.h>

void BermudaStackInit(t, sp, stack_size, handle)
THREAD *t;
stack_t sp;
unsigned short stack_size;
thread_handle_t handle;
{
        /* if the stack pointer is NULL we will setup the main stack */
        if(NULL == sp)
                sp = (stack_t)MEM-stack_size-2;

        t->stack = sp;
        t->stack_size = stack_size;
        t->sp = &sp[stack_size-1];
        
        /*
         * first we add the function pointer to the stack
         */
        *(t->sp--) = (unsigned short)handle & 0xff;
        *(t->sp--) = ((unsigned short)handle >> 8) & 0xff;
        
        /* add the SREG register */
        *(t->sp--) = 0x0; // location of R0 normally
        *(t->sp--) = *AvrIO->sreg;
        
        /* pad the other registers */
        int i = 0;
        for(; i < 31; i++)
                *(t->sp--) = 0;
#if defined(__THREAD_DBG__) && (__VERBAL__)
        printf("Stack: %p - Param: %p\n", t->stack, t->param);
#endif
        t->sp[8] = ((unsigned short)t->param) & 0xFF;
        t->sp[7] = (((unsigned short)t->param) >> 8) & 0xFF;
}

void BermudaStackSave(stack_t sp)
{
        if(BermudaCurrentThread == NULL)
                return;

        sp += 2;
        BermudaCurrentThread->sp = sp;
        // switch the current thread pointer
        BermudaCurrentThread = BermudaRunQueue;
        BermudaCurrentThread->state = THREAD_RUNNING;
}
#endif
