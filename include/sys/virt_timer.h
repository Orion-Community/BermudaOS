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

//! \addtogroup vtimers
// @{

struct _vtimer;

/**
 * \typedef vtimer_callback
 * \brief Call-back function type for virtual timers.
 */
typedef void (*vtimer_callback)(struct _vtimer*, void * arg);

#define TIMER_DEF(fn, arg1, arg2) \
PUBLIC void fn(struct _vtimer *arg1, void *arg2)

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
         * \brief Amount of ticks left.
         * 
         * Amount of system ticks left before the handle will be called.
         */
        unsigned long ticks_left;
        
        /**
         * \brief Timer ticks.
         * \see ticks_left
         * \note This member will be set to 0 when it is a one-shot timer.
         * 
         * Interval of the timer.
         */
        unsigned long ticks;
        
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

/**
 * \def BermudaTimerDelete(t)
 * \brief Mark a timer as done.
 * \warning The timer will not be deleted, but it should not be considered as
 * existing after using BermudaTimerDelete.
 * 
 * This def will not actually delete the timer, but it will mark it as deletable.
 * The actual deletion will be done by BermudaVirtualTick.
 */
#define BermudaTimerDelete(t) (t->interval = 0)

#ifdef __cplusplus
extern "C" {
#endif

extern void BermudaTimerProcess();
extern VTIMER *BermudaTimerCreate(unsigned long ms, vtimer_callback fn, void *arg,
                                     unsigned char flags);
extern void BermudaTimerStop(VTIMER *timer);
extern void BermudaTimerInit();
extern void BermudaDelay(unsigned short ms);
extern void BermudaDelay_us(unsigned long us);

// internal functions
PRIVATE WEAK void BermudaTimerAdd(VTIMER *timer);

#ifdef __cplusplus
}
#endif

// @}

#endif /* __VTIMER_H */