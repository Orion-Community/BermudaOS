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

ACTION_EVENT_DECL(dummy, arg);

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

/**
 * \fn BermudaEventRootAdd(EVENT *e)
 * \brief Add an event to the root.
 * \param e Event to add.
 * \see BermudaEventAdd
 * 
 * The given event <i>e</i> will be added to the childeren of the root event.
 */
void BermudaEventRootAdd(EVENT *e)
{
        BermudaEventAdd(_event_queue, e);
}

/**
 * \fn BermudaEventAdd(EVENT *parent, EVENT *e)
 * \brief Add an event to the tree.
 * \param parent Parent node.
 * \param e      Event to add to the tree.
 * 
 * Add the given event <i>e</i> to the tree (as child to the given <i>
 * parent</i>).
 */
PRIVATE WEAK void BermudaEventAdd(EVENT *parent, EVENT *e)
{
        if(NULL == parent->child)
                parent->child = e;
        else
        { // iterate trough the childeren of parent
                EVENT *c = parent->child;
                for(; c != NULL && c != c->next; c = c->next)
                {
                        if(c->next == NULL)
                        {
                                c->next = e;
                                break;
                        }
                }
        }
        return;
}

/**
 * \fn signal()
 * \brief Release the semaphore.
 * \param e Event to release.
 * 
 * This function releases the current semaphore.
 */
PRIVATE WEAK void signal(EVENT *e, int status)
{}

THREAD(BermudaEventExec, data)
{
        EVENT *e = (EVENT*)data;
        int status = e->handle(e);
        signal(e, status);
        BermudaThreadExit();
}

/**
 * \fn ACTION_EVENT(dummy, arg)
 * \brief Dummy action event.
 * 
 * Preprocesses to: <i>PRIVATE WEAK void dummy(void *arg)</i>. This is the dummy
 * action event, which will be executed if there is no other action event
 * available.
 */
ACTION_EVENT(dummy, arg)
{
        return 0;
}
#endif /* __EVENTS__ */
