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

/** \file virt_timer.c */

#include <bermuda.h>
#include <sys/virt_timer.h>
#include <arch/io.h>

static unsigned long last_sys_tick;

/**
 * \addtogroup vtimers Timer Management API
 * \brief Timer management API.
 * 
 * API to handle timers which run in the back ground. An example of the use of
 * these timers is in the BermudaThreadSleep function. The scheduler uses them
 * to clock sleep time's.
 * 
 * @{
 */

/**
 * \var BermudaTimerList
 * \brief Timer linked list.
 * \see BermudaTimerAdd
 * \private
 * 
 * Linked list of timer objects. The list is in control of the timer module.
 */
PRIVATE WEAK VTIMER *BermudaTimerList = NULL;

/**
 * \var delay_loop_count
 * \brief Delay loops.
 * \see BermudaTimerInit
 * \private
 * 
 * Amount of loops per system tick.
 */
PRIVATE WEAK unsigned long delay_loop_count = 0;

/**
 * \brief Initialise the timer module.
 * \note Called by the BermudaOS initialisation sequence.
 * \warning Should never be called by any user application.
 * \see delay_loop_count
 * 
 * Registrates and starts a hardware timer. Also the delay_loop_count will
 * be intialised.
 */
PUBLIC void BermudaTimerInit()
{
        unsigned long count = BermudaTimerGetSysTick();
        while(count == BermudaTimerGetSysTick())
                delay_loop_count++;

        /*
         * The loop above takes more cycles due to the function call in the
         * while loop. So a correction has to be applied.
         */
#ifdef __AVR__
        delay_loop_count *= 103UL;
        delay_loop_count /= 26UL;
#else
        delay_loop_count *= 137UL;
        delay_loop_count /= 25UL;
#endif
}

/**
 * \brief Delay for given amount of micro seconds.
 * \param us Amount of micro seconds to delay.
 * \todo Create a NOP definition for arch independency.
 * 
 * The CPU will busy wait for the given amount of micro seconds.
 */
PUBLIC void BermudaDelay_us(unsigned long us)
{
        register unsigned long count = delay_loop_count * us / 1000;
        
        while(count--)
                __asm__ __volatile__("mov r0, r0");
}

/**
 * \brief Delay for given amount of mili seconds.
 * \param us Amount of mili seconds to delay.
 * 
 * The CPU will busy wait for the given amount of mili seconds.
 */
PUBLIC void BermudaDelay(unsigned char ms)
{
        BermudaDelay_us((unsigned long)ms * 1000);
}

/**
 * \brief Create a new timer.
 * \param ms Time until it will fire in milli seconds.
 * \param fn Fire function.
 * \param arg Argument passed to <b>fn</b>.
 * \param flags Can be set to either BERMUDA_PERIODIC or BERMUDA_ONE_SHOT
 * \return The created timer object.
 * \see BERMUDA_ONE_SHOT
 * \see BERMUDA_PERIODIC
 * \see BermudaSchedulerExec
 * \note Each time BermudaSchedulerExec is called the timer list is processed.
 *       If threads are not available, they will be handled in the timer interrupt.
 * \todo Rewrite for new implementation.
 * 
 * Create's a new timer object based on the given parameters. When the timer
 * expires it will call the given function <b>fn</b>.
 */
PUBLIC VTIMER *BermudaTimerCreate(unsigned long ms, vtimer_callback fn, void *arg,
                                     unsigned char flags)
{
        VTIMER *timer;
        if((timer = BermudaHeapAlloc(sizeof(*timer))) != NULL)
        {
                unsigned long ticks = BermudaTimerMillisToTicks(ms);
                if(flags & BERMUDA_ONE_SHOT)
                        timer->ticks = 0;
                else
                        timer->ticks = ticks;
                
                timer->ticks_left = ticks+BermudaTimerGetSysTick()-last_sys_tick;
                timer->handle = fn;
                timer->arg = arg;
                timer->next = NULL;
                BermudaTimerAdd(timer);
        }
        return timer;
}

/**
 * \brief Stop a running timer.
 * \param timer Timer to stop.
 * \note This function will not free any memory. This will be done by 
 *       BermudaTimerProcess.
 * \see BermudaTimerProcess
 * 
 * The handle, ticks and ticks_left will be set to zero. This will cause 
 * BermudaTimerProcess to stop the timer from executing.
 */
PUBLIC void BermudaTimerStop(VTIMER *timer)
{
        VTIMER *tqp = BermudaTimerList, *prev = NULL;
        timer->handle = NULL;
        timer->ticks = 0;

        if(timer->ticks_left)
        { // if not yet elapsed
                while(tqp)
                {
                        if(tqp == timer)
                                break;
                        
                        prev = tqp;
                        tqp = tqp->next;
                }
                
                if(prev)
                        prev->next = timer->next;
                else
                        BermudaTimerList = timer->next;
                
                if(timer->next)
                        timer->next->ticks_left += timer->ticks_left;
                
                timer->ticks_left = 0;
                BermudaHeapFree(timer);
        }
}

/**
 * \brief Add the given timer to the list.
 * \param timer New timer to add.
 * \warning Be careful not to add a timer twice.
 * \see _vtimer
 * \private
 * \todo Fix head == NULL cases
 * 
 * The timer will be added to the list of timers. The list is ordered by the
 * <b>ticks_left</b> field.
 */
PRIVATE WEAK void BermudaTimerAdd(VTIMER *timer)
{
        VTIMER *vtp = BermudaTimerList, *prev = NULL;
        
        while(vtp)
        {
                if(timer->ticks_left < vtp->ticks_left)
                {
                        vtp->ticks_left -= timer->ticks_left;
                        break;
                }
                
                timer->ticks_left -= vtp->ticks_left;;
                
                prev = vtp;
                vtp = vtp->next;
        }
        
        timer->next = vtp;
        
        if(prev)
                prev->next = timer;
        else
                BermudaTimerList = timer;
}

/**
 * \fn BermudaTimerProcess()
 * \brief Process all virtual timers.
 * 
 * Update all virtual timers. This action is done before switching context by
 * default.
 */
PUBLIC void BermudaTimerProcess()
{
        VTIMER *timer = NULL;
        unsigned long new_ticks = BermudaTimerGetSysTick(); // save cycles by
                                                           // saving in a local var
        
        unsigned long diff = new_ticks - last_sys_tick;
        last_sys_tick = new_ticks;
        
        /*
         * Loop trough the list of timers. As long as there is a timer list and
         * there are ticks left to distribute.
         */
        while(diff && BermudaTimerList)
        {
                timer = BermudaTimerList;
                if(timer->ticks_left < diff)
                {
                        diff -= timer->ticks_left;
                        timer->ticks_left = 0;
                }
                else
                {
                        timer->ticks_left -= diff;
                        diff = 0;
                }
                
                // timer elapsed when ticks_left == 0
                if(timer->ticks_left == 0)
                {
                        if(timer->handle)
                                timer->handle(timer, timer->arg);
                        
                        BermudaTimerList = timer->next;
                        timer->ticks_left = timer->ticks;
                        if(timer->ticks_left == 0)
                                BermudaHeapFree(timer);
                        else
                                BermudaTimerAdd(timer);
                }
        }
}

// @}
