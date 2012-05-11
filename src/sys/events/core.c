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

#ifdef __EVENTS__

#ifndef __THREADS__
#       error Handling of events is not possible without threading.
#endif

#include <bermuda.h>

#include <arch/io.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/events/core.h>

/**
 * \fn BermudaEventInit()
 * \brief Initialse an event.
 * \param e The event queue.
 * \param type The event queue type.
 * \see struct _event
 * 
 * This function will initialse the event frame work by allocating the the
 * event queue.
 */
PRIVATE WEAK void BermudaEventInit(EVENT *e, EVENT_TYPE type)
{
        if(NULL != e)
                return;
        
        e = BermudaHeapAlloc(sizeof(*e)); // allocate one
                                                                // entry.
        e->type = type;
        e->max_wait = BERMUDA_EVENT_WAIT_INFINITE;
        e->thread = BermudaSchedGetIdleThread();
        return;
}

/**
 * \fn BermudaEventTick()
 * \brief One ms tick.
 *
 * This function clocks all events.
 */
void BermudaEventTick()
{
}
#endif /* __EVENTS__ */
