/*
 *  BermudaOS - Thread module
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
#include <stdio.h>

#include <kernel/thread.h>
#include <kernel/stack.h>
#include <kernel/ostimer.h>

PUBLIC struct thread *thread_create(name, handle, arg, stack_size, stack, prio)
char *name;
thread_handle_t handle;
void *arg;
size_t stack_size; 
void *stack;
uint8_t prio;
{
	struct thread *t = malloc(sizeof(*t));
	
	t->name = name;
	t->handle = handle;
	t->arg = arg;
	t->priority = prio;
	
	stack_init(t, stack_size, stack);
	thread_run_queue_add(t);
	return t;
}