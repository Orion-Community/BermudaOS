/*
 *  BermudaOS - Threading module
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

/** \file thread.c */

#include <string.h>
#include <stdlib.h>

#include <avr/io.h>
#include <arch/avr/io.h>

#include <sys/thread.h>

/**
 * \fn BermudaThreadInit(THREAD *t, thread_handle_t handle, void *arg,
 *                               unsigned short stack_size, void *stack)
 * \brief Initialize the scheduler with main thread.
 * \param t Main thread
 * \param handle Main handle
 * \param arg Arguments to the main thread
 * \param stack_size Size of the stack
 * \param stack Stack pointer
 * 
 * Initialize the main thread. If <i>stack</i> is NULL, then the current stack
 * pointer will be used.
 */
int BermudaThreadInit(THREAD *t, thread_handle_t handle, void *arg,
                                unsigned short stack_size, void *stack)
{
        if(NULL == t)
                return -1;
                
        if(NULL == stack)
                stack = (void*)(BermudaGetStackPointer() - stack_size - 2);
        
        t->stack = stack;
        t->stack_size = stack_size;
        t->param = arg;
        t->sp = (unsigned char*)(&t->stack[stack_size-1]);
        
        return 0;
}
