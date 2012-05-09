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
#include <sys/event.h>

PRIVATE WEAK void dummy(void *arg);

/**
 * \var _event_queue
 *
 * Tree of all events.
 */
EVENT *_event_queue = NULL;

/**
 * \fn BermudaEventInit()
 * \brief Initialise the event frame work.
 * \see _event_queue
 * 
 * This function will initialse the event frame work by allocating the the
 * event queue.
 */
PRIVATE WEAK void BermudaEventInit()
{
        if(NULL != _event_queue)
                return;
        
        _event_queue = BermudaHeapAlloc(sizeof(*_event_queue)); // allocate one
                                                                // entry.
        // initialse parents, childs and next to NULL
        _event_queue->next = NULL; _event_queue->parent = NULL; _event_queue->child = NULL;
        _event_queue->type = EVENT_ROOT;
        _event_queue->max_wait = BERMUDA_EVENT_WAIT_INFINITE;
        _event_queue->thread = BermudaSchedGetIdleThread();
        _event_queue->handle = &dummy;
        _event_queue->arg = NULL;
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

ACTION_EVENT(dummy, arg)
{
        return;
}
#endif /* __EVENTS__ */