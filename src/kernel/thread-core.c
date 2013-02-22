/*
 *  BermudaOS - Thread core
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

#include <dev/dev.h>
#include <dev/error.h>

/**
 * \brief Currently running threads.
 * 
 * Each CPU core has its own running thread.
 */
static struct thread *current_thread[CPU_CORES];

/**
 * \brief The idle thread definition.
 */
static struct thread idle_thread;
/**
 * \brief The main thread definition.
 */
static struct thread main_thread;

static uint64_t thread_id_counter = 0;

/**
 * \brief Generate a new thread ID.
 */
static inline uint64_t thread_generate_thread_id()
{
	uint64_t i = thread_id_counter;
	
	thread_id_counter += 1;
	return i;
}

/**
 * \brief Initialize a new thread information structure and its stack.
 * \param t Thread to initialize.
 * \param stack Stack pointer.
 * \param size Size of \p stack.
 * \note If \p stack is \p NULL, a stack will be allocated.
 */
PUBLIC int thread_add_new(struct thread *t, void *stack, size_t stack_size)
{
	int rc = -DEV_OK;
	
	if(stack == NULL) {
		stack = malloc(stack_size);
		return -DEV_NULL;
	}
	
	rc = stack_init(t, stack_size, stack);
	t->id = thread_generate_thread_id();
	
	return rc;
}
