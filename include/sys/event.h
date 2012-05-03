/*
 *  BermudaOS - Lockable events
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

/** \file event.h */
#ifndef __EVENT_H_
#define __EVENT_H_

#include <bermuda.h>

#include <arch/io.h>

/**
 * \def BERMUDA_EVENT_WAIT_INFINITE
 * 
 * Wait for an event for an infinite amount of time.
 */
#define BERMUDA_EVENT_WAIT_INFINITE 0

__DECL
/**
 * \fn BermudaEventWait(THREAD *queue, unsigned int mdt)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param mdt <i>Maximum Delay Time</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
extern void BermudaEventWait(THREAD *queue, unsigned int mdt);

/**
 * \fn BermudaEventPost(THREAD *queue)
 * \brief Release the current event and execute the next event in the queue.
 * \param queue Event queue.
 * 
 * Release the lock of the current event and add the next event to the sched queue.
 */
extern void BermudaEventPost(THREAD *queue);

/**
 * \fn signal()
 * \brief Release the semaphore.
 * 
 * This function releases the current semaphore.
 */
PRIVATE WEAK void signal();

/**
 * \fn wait()
 * \brief Lock the semaphore.
 * 
 * Lock the current counting semaphore.
 */
PRIVATE WEAK void wait();
__DECL_END

#endif