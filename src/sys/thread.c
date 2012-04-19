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

#include <stdlib.h>

#include <arch/io.h>
#include <arch/stack.h>

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
                                unsigned short stack_size, void *stack,
                                unsigned char prio)
{
        if(NULL == t)
                return -1;

        BermudaStackInit(t, stack, stack_size, handle);
        t->param = arg;
        t->prio = prio;
        t->flags = 0;
        
        return 0;
}
