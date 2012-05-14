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

struct _vtimer;

/**
 * \typedef vtimer_callback
 * \brief Call-back function type for virtual timers.
 */
typedef void (*vtimer_callback)(struct _vtimer*, void * arg);

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
        void (*handle)(struct _vtimer*, void * arg);
        
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
         * is 1ms (or 1000Hz).
         */
        unsigned long interval;
        
        /**
         * \brief Timer ticks.
         * \see interval
         * 
         * The amount of counted ticks. Each interval, this member will be increased
         * by one.
         */
        unsigned long ticks;
        
        /**
         * \brief Timer flags.
         * 
         * Set to BERMUDA_ONE_SHOT to create a timer which will only fire once.
         * When set to BERMUDA_PERIODIC, it will fire at every interval until
         * stopped.
         */
        unsigned char flags;
} __PACK__;

/**
 * \typedef VTIMER
 * \brief Virtual timer type.
 * \see struct _vtimer
 */
typedef struct _vtimer VTIMER;

/**
 * \def BERMUDA_ONE_SHOT
 * \brief One shot timer.
 */
#define BERMUDA_ONE_SHOT 1

/**
 * \def BERMUDA_PERIODIC
 * \brief Periodic timer.
 */
#define BERMUDA_PERIODIC 0

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

/**
 * \fn BermudaTimerCreate(unsigned int ms, vtimer_callback fn, void *arg,
                                     unsigned char flags)
 * \brief Create a new virtual timer.
 * \param ms Timer interval.
 * \param fn Function call-back.
 * \param arg Call-back argument.
 * \param flags Timer flags. When set to BERMUDA_ONE_SHOT, the timer will only trigger once.
 * \return NULL when an error occurs. The created timer object on success.
 * \see _vtimer
 * 
 * This function creates a timer and the timer is started immediatly. This function
 * will return after creation and the timer runs in the back ground.
 */
extern VTIMER *BermudaTimerCreate(unsigned int ms, vtimer_callback fn, void *arg,
                                     unsigned char flags);

/**
 * \fn BermudaVTimerAdd(VTIMER *timer)
 * \brief Add a new virtual timer to the list.
 * \param timer The timer to add.
 * \see _vtimer
 * 
 * The given virtual timer will be added to the list of virtual timers. Doing so
 * will start the timer.
 */
PRIVATE WEAK void BermudaVTimerAdd(VTIMER *timer);

#ifdef __cplusplus
}
#endif

#endif /* __VTIMER_H */