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
        /** \brief Root event. */
        EVENT_ROOT,
        /** \brief Virtual event */
        EVENT_VIRTUAL,
        /** \brief I/O event */
        EVENT_IO, 
        /** \brief I2C event. Must be a child of an I/O event. */
        EVENT_I2C,
        /** \brief SPI event. Must be a child of an I/O event. */
        EVENT_SPI,
} EVENT_TYPE;

/**
 * \typedef void (*action_event)(void*)
 * \brief Type for action handlers.
 * 
 * A function of this type will be called when an event triggered.
 */
typedef void (*action_event)(void*);

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
         * \note NULL means end of list.
         * \warning Should never point to itself.
         * 
         * Pointer to next entry in the list.
         */
        struct _event *next;
        
        /**
         * \brief Parent pointer.
         * \note A NULL parent means this node is the root node.
         * 
         * This points to the parent node.
         */
        struct _event *parent;
        
        /**
         * \brief Childeren pointer.
         * 
         * Points to the childeren of this node.
         */
        struct _event *child;

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
 * \var _event_queue
 *
 * Tree of all events.
 */
extern EVENT *_event_queue;

/**
 * \def BermudaGetEventQueue
 *
 * Get the global tree of events.
 */
#define BermudaGetEventQueue() _event_queue

#define ACTION_EVENT(fn, arg0) \
PRIVATE WEAK void fn(void * arg0); \
PRIVATE WEAK void fn(void * arg0)


/**
 * \fn BermudaEventTick()
 * \brief One ms tick.
 *
 * This function clocks all events.
 */
__DECL
extern void BermudaEventTick();

/**
 * \fn BermudaEventInit()
 * \brief Initialise the event frame work.
 * \see _event_queue
 * 
 * This function will initialse the event frame work by allocating the the
 * event queue.
 */
PRIVATE WEAK void BermudaEventInit();

/**
 * \fn BermudaEventRootAdd(EVENT *e)
 * \brief Add an event to the root.
 * \param e Event to add.
 * \see BermudaEventAdd
 * 
 * The given event <i>e</i> will be added to the childeren of the root event.
 */
extern void BermudaEventRootAdd(EVENT *e);

/**
 * \fn BermudaEventAdd(EVENT *parent, EVENT *e)
 * \brief Add an event to the tree.
 * \param parent Parent node.
 * \param e      Event to add to the tree.
 * 
 * Add the given event <i>e</i> to the tree (as child to the given <i>
 * parent</i>).
 */
PRIVATE WEAK void BermudaEventAdd(EVENT *parent, EVENT *e);

/**
 * \fn BermudaEventWait(EVENT **queue, unsigned int mdt)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param mdt <i>Maximum Delay Time</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
extern void BermudaEventWait(EVENT **queue, unsigned int mdt);

/**
 * \fn BermudaEventPost(EVENT *queue)
 * \brief Release the current event and execute the next event in the queue.
 * \param queue Event queue.
 * 
 * Release the lock of the current event and add the next event to the
 * sched queue.
 */
extern void BermudaEventPost(EVENT *queue);

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