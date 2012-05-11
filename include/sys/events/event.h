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

#include <bermuda.h>

/**
 * \class Event
 * \brief Class representing the event objects.
 * 
 * Objects of this class are events which can be triggered by other subsystems
 * and are executable.
 */
class Event
{
public:
        Event();
        
        /**
         * \fn signal()
         * \brief Release the semaphore.
         * 
         * This function releases the current semaphore.
         */
        void signal();
        
        /**
         * \fn wait()
         * \brief Lock the semaphore.
         * 
         * Lock the current counting semaphore.
         */
        void wait();
        
        /**
         * \fn GetEventState()
         * \brief Request the current state of an event.
         * \return The state of the event.
         * 
         * This method returns the current state of an event.
         */
        int  GetEventState();
        
private:
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
