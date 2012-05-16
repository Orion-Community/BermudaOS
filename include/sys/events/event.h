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
#include <sys/thread.h>
#include <sys/virt_timer.h>

/**
 * \def BERMUDA_EVENT_WAIT_INFINITE
 * \brief Wait infinite.
 * 
 * The thread will wait infinite for a passing event.
 */
#define BERMUDA_EVENT_WAIT_INFINITE 0

/**
 * \def SIGNALED
 * \brief Signaled state.
 * 
 * A queue will have the signaled state when an event is posted to the queue
 * when there are no events waiting.
 */
#define SIGNALED ((void*)-1)

/**
 * \typedef EVENT
 * \brief Event type.
 * 
 * Events are nothing more than a list of threads, typed as void pointers.
 */
typedef void* EVENT;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \fn BermudaEventWait(volatile THREAD **queue, unsigned int tmo)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param tmo <i>Time out</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
extern int BermudaEventWait(volatile THREAD **queue, unsigned int tmo);

/**
 * \fn BermudaEventSignal(volatile EVENT *)
 * \brief Signal the given event queue.
 * 
 * Signal the given event queue.
 */
extern void BermudaEventSignal(volatile EVENT *);

/**
 * \fn BermudaEventTMO(VTIMER *timer, void *arg)
 * \brief Timeout function.
 * \param timer Timer object which called this function.
 * \param arg Void casted argument of the event queue, where a thread received a
 * timeout in.
 * 
 * When a thread timeouts waiting for an event, the wait function will return with
 * an error.
 */
PRIVATE WEAK void BermudaEventTMO(VTIMER *timer, void *arg);

__DECL_END

#endif