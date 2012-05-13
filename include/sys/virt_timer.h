/*
 *  BermudaOS - Virtual timers
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

/** \file virt_timer.h */

#ifndef __VTIMER_H
#define __VTIMER_H

#include <bermuda.h>

/**
 * \struct _vtimer
 * \brief Virtual timer structure.
 * 
 * Virtual timers run in the back ground to clock certain processes.
 */
struct _vtimer
{
        /**
         * \brief Next pointer.
         * 
         * Pointer to the next virtual timer.
         */
        struct _vtimer *next;
        
        /**
         * \brief Call-back.
         * 
         * Function which will be called each tick.
         */
        void (*handle)(struct _vtimer, void * arg);
        
        /**
         * \brief Timer tick call-back argument.
         * \see handle
         * 
         * Argument passed to the call-back handle.
         */
        void *arg;
        
        /**
         * \brief Timer interval.
         * 
         * The interval of each tick. The base interval (ie. when interval = 1)
         * is 0.5ms (or 2000Hz).
         */
        unsigned long interval;
        
        /**
         * \brief Timer ticks left.
         * 
         * The amount of ticks left before the timer expires. This member will
         * be decremented each interval.
         */
        unsigned long ticks_left; //!< Amount of ticks left
} __PACK__;

/**
 * \typedef VTIMER
 * \brief Virtual timer type.
 * \see struct _vtimer
 */
typedef struct _vtimer VTIMER;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \fn BermudaVirtualTick()
 * \brief Virtual timer tick.
 * 
 * Give all registered virtual timers one system tick, and call their call back
 * function.
 */
extern void BermudaVirtualTick();

#ifdef __cplusplus
}
#endif

#endif /* __VTIMER_H */