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

typedef enum
{
	THREAD_RUNNING,
	THREAD_READY,
	THREAD_SLEEPING,
} thread_state_t;

struct thread
{
	struct thread *next; //!< Next pointer in the total list of threads.
	struct thread *q_next; //!< Next queue pointer.
	struct thread *volatile*queue; //!< Pointer pointer to the current queue.
	
	char *name;
	uint64_t id;
	
	struct stack *stack;
	thread_state_t state;
	uint8_t event_counter;
	uint8_t priority;
	uint16_t sleep_time;
};

#endif
