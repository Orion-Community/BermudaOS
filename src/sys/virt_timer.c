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

void BermudaTimerDelete(VTIMER *timer)
{
        timer->interval = 0;
}

/**
 * \fn BermudaVirtualTick()
 * \brief Virtual timer tick.
 * 
 * Give all registered virtual timers one system tick, and call their call back
 * function.
 */
void BermudaVirtualTick()
{
        if(head == NULL)
                return;
        
        VTIMER *c = head;
        VTIMER *prev = NULL;
        while(c)
        {
                c->ticks++;
                if((c->ticks % c->interval) == 0)
                {
                        c->handle(c, c->arg);
                        if(c->interval == 0)
                        { // delete the timer
                                if(prev == NULL)
                                {
                                        head = c->next;
                                        BermudaHeapFree(c);
                                        c = head;
                                        continue;
                                }
                                else
                                {
                                        prev->next = c->next;
                                        BermudaHeapFree(c);
                                        c = prev->next;
                                        continue;
                                }
                        }
                }
                
                prev = c;
                if(c->next == NULL)
                        break;
                else
                        c = c->next;
        }
}