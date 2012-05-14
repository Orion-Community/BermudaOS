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
 * \fn BermudaEventQueueAdd(EVENT *queue, EVENT *e)
 * \brief Add an event to a queue.
 * \param queue The event queue.
 * \param e The event to add to the queue.
 * 
 * Add the <i>e</i> to the given queue <i>queue</i>.
 */
PRIVATE WEAK void BermudaEventQueueAdd(EVENT *queue, EVENT *e);

/**
 * \fn BermudaEventWait(EVENT *queue, unsigned int mdt)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param mdt <i>Maximum Delay Time</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
extern void BermudaEventWait(EVENT *queue, unsigned int mdt);

/**
 * \fn BermudaEventSignal(EVENT *)
 * \brief Signal the given event queue.
 * 
 * Signal the given event queue.
 */
PRIVATE WEAK void BermudaEventSignal(EVENT *);

__DECL_END

#endif