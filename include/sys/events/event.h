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

//! \addtogroup event_management
// @{

/**
 * \def BERMUDA_EVENT_WAIT_INFINITE
 * \brief Wait infinite.
 * 
 * The thread will wait infinite for a passing event.
 */
#define BERMUDA_EVENT_WAIT_INFINITE 0

/**
 * \def EVENT_WAIT_INFINITE
 * \brief Wait infinite.
 * 
 * The thread will wait infinite for a passing event.
 */
#define EVENT_WAIT_INFINITE BERMUDA_EVENT_WAIT_INFINITE

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

#ifdef __EVENTS__
extern int BermudaEventWait(volatile THREAD **queue, unsigned int tmo);
extern int BermudaEventSignal(volatile THREAD **tqpp);
extern int BermudaEventSignalRaw(THREAD *volatile*tqpp);
extern void BermudaEventSignalFromISR(volatile THREAD **tqpp);
extern int BermudaEventWaitNext(volatile THREAD **tqpp, unsigned int tmo);

// private functions
PRIVATE WEAK void BermudaEventTMO(VTIMER *timer, void *arg);
#endif

__DECL_END

#define event_wait(q, tmo) BermudaEventWait(q, tmo)
#define event_signal(q) BermudaEventSignal(q)

// @}
#endif