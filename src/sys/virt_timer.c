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

PRIVATE WEAK VTIMER *head = NULL;

VTIMER *BermudaTimerCreate(unsigned int ms, vtimer_callback fn, void *arg,
                                     unsigned char flags)
{
        VTIMER *timer = BermudaHeapAlloc(sizeof(*timer));
        if(NULL == timer)
                return NULL;
                
        timer->interval = ms;
        timer->handle = fn;
        timer->arg = arg;
        timer->flags = flags;
        BermudaVTimerAdd(timer);
        return timer;
}

PRIVATE WEAK void BermudaVTimerAdd(VTIMER *timer)
{
        timer->next = NULL;
        
        if(head == NULL)
                head = timer;
        else
        {
                VTIMER *c = head;
                while(c)
                {
                        if(NULL == c->next)
                                break;
                        
                        c = c->next;
                }
                c->next = timer;
        }
        
        return;
}

/**
 * \brief Delete a timer.
 * \param timer Timer to delete.
 * \see BermudaTimerProcess
 * 
 * When a timer expired or is not needed anymore, it can be stopped and deleted
 * by this thread.
 */
void BermudaTimerExit(VTIMER *timer)
{
}

/**
 * \fn BermudaTimerProcess()
 * \brief Process all virtual timers.
 * 
 * Update all virtual timers. This action is done before switching context by
 * default.
 */
void BermudaTimerProcess()
{
}