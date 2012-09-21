/*
 *  BermudaOS - Action events
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

/** \file action_event.h */

#ifndef __ACTION_EVENT_H
#define __ACTION_EVENT_H

#include <bermuda.h>

#include <sys/sched.h>
#include <sys/thread.h>

#define ACTION_EVENT(fn, arg0) \
PRIVATE WEAK int fn(void * arg0)

#define ACTION_EVENT_DECL(fn, arg0) \
PRIVATE WEAK int fn(void * arg0);


/**
 * \struct _event
 * \brief Event data type.
 *
 * Definies the event data structure, which can be executed by the scheduler.
 */
struct _action_event
{
        /**
         * \brief Next pointer.
         * 
         * Pointer to the next node in the action event list/queue.
         */
        struct _action_event *next;

        /**
         * \brief Event handler.
         * \note If the handle equals NULL, the action_event of the parent node
         *       will be executed.
         * 
         * This function will be run as a RUN_ONCE_THREAD.
         */
        action_event handle;
        
        /**
         * \brief Event handler arguments.
         *
         * Arguments to pass to the action event handler.
         */
        void *arg;
        
        /**
         * \brief Associated thread.
         * 
         * The thread which is associated with this action event.
         */
        THREAD *thread;
};

#endif
