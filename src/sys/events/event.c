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

/** \file event.c */

#if defined(__EVENTS__) || defined(__DOXYGEN__)

#ifndef __THREADS__
#       error Handling of events is not possible without threading.
#endif

#include <bermuda.h>

#include <arch/io.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/events/event.h>

/**
 * \fn BermudaEventWait(volatile THREAD **queue, unsigned int tmo)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param tmo <i>Time out</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
PUBLIC int BermudaEventWait(volatile THREAD **queue, unsigned int tmo)
{

}

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
PRIVATE WEAK void BermudaEventTMO(VTIMER *timer, void *arg)
{

}


#endif /* __EVENTS__ */