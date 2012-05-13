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

/** \file core.h */
#ifndef __EVENT_H_
#define __EVENT_H_

#include <bermuda.h>

#include <arch/io.h>
#include <sys/thread.h>
#include <sys/virt_timer.h>

/**
 * \def BERMUDA_EVENT_WAIT_INFINITE
 * 
 * Wait for an event for an infinite amount of time.
 */
#define BERMUDA_EVENT_WAIT_INFINITE 0

/**
 * \enum _event_type
 * \typedef EVENT_TYPE
 * \brief Event type definition.
 * \see enum _event_type
 * 
 * This enumeration defines the different types of events.
 */
typedef enum _event_type
{
        BERMUDA_EVENT_TYPE,
        BERMUDA_ACTION_EVENT_TYPE
} EVENT_TYPE;

/**
 * \typedef int (*action_event)(void*)
 * \brief Type for action handlers.
 * 
 * A function of this type will be called when an event triggered.
 */
typedef int (*action_event)(void*);

/**
 * \struct _event
 * \brief Event data type.
 *
 * Definies the event data structure, which can be executed by the scheduler.
 */
struct _event
{
        /**
         * \brief Next pointer.
         * 
         * Pointer to the next in the list.
         */
        struct *_event next;
        
        /**
         * \brief Event timer.
         * \see _vtimer
         * 
         * This timer will make sure the event returns an error when max_wait
         * elapses.
         */
        VTIMER *timer;
        
        /**
         * \brief Maximum time to wait.
         * 
         * Maximum time the event should wait. If the event didn't take place,
         * the event will be deleted.
         */
        unsigned int max_wait;
        
        /**
         * \brief Associated thread.
         *
         * The thread associated with this event.
         */
        THREAD *thread;
        
        /**
         * \brief Type of this event.
         * 
         * A type describing this event.
         */
        EVENT_TYPE type;
};

/**
 * \typedef EVENT
 * \brief Type definition of struct _event.
 * \see struct _event
 */
typedef struct _event EVENT;

/**
 * \fn BermudaEventCreate()
 * \brief Initialise the event frame work.
 * \see _event_queue
 * 
 * This function will initialse the event frame work by allocating the the
 * event queue.
 */
__DECL
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
 * \fn signal()
 * \brief Signal the given event queue.
 * 
 * Signal the given event queue.
 */
PRIVATE WEAK void BermudaEventSignal(EVENT *);

extern void BermudaEventDebug();
__DECL_END

#endif