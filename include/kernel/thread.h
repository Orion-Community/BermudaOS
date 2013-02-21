/*
 *  BermudaOS - Thread header
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

#ifndef __THREAD_H_
#define __THREAD_H_

#include <lib/binary.h>

#include <kernel/event.h>
#include <kernel/stack.h>

#define THREAD_SLEEPING_SHIFT 0
#define THREAD_RUNNING_SHIFT 1
#define THREAD_READY_SHIFT 2
#define THREAD_KILLED_SHIFT 3

#define THREAD_SLEEPING_MASK BIT(THREAD_SLEEPING_SHIFT)
#define THREAD_RUNNING_MASK BIT(THREAD_RUNNING_SHIFT)
#define THREAD_READY_MASK BIT(THREAD_READY_SHIFT)
#define THREAD_KILLED_MASK BIT(THREAD_KILLED_SHIFT)

typedef void (*thread_handle_t)(void *param);

/**
 * \brief Thread structure.
 * 
 * This structure defines the current state of a thread.
 */
struct thread
{
	struct thread *left; //!< Left pointer in the red black tree.
	struct thread *right; //!< Right pointer in the red black tree.
	struct thread *next; //!< Next queue pointer.
	struct thread *volatile*queue; //!< Pointer pointer to the current queue.
	uint64_t cpu_time; //!< Amount of time (in ms) this thread has been using the CPU.
	
	char *name; //!< Unique thread name.
	uint64_t id; //!< Unique thread identifier.
	
	struct event *event; //!< Event attached to this thread.
	struct stack *stack; //!< Stack structure.
	uint8_t ec; //!< Event counter.
	uint16_t sleep_time; //!< Time to sleep left.
	uint8_t priority; //!< Priority of this thread.
	thread_handle_t handle; //!< The thread handle.
	void *arg; //!< Argument past to the thread.
	
	/* current state */
	uint8_t sleeping : 1; //!< If set to one, this thread is sleeping.
	uint8_t running : 1; //!< If set to one, this thread is running.
	uint8_t ready : 1; //!< If set to one, this thread is ready to run.
	uint8_t killed : 1; //!< If set to one, this thread is about to be killed.
	uint8_t signaled : 1; //!< This thread received an event signal.
} __attribute__((packed));

extern int thread_run_queue_add(struct thread *t);
#endif
