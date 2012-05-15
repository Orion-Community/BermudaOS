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

/** \file event.c */

#if defined(__EVENTS__) || defined(__DOXYGEN__)

#ifndef __THREADS__
#       error Handling of events is not possible without threading.
#endif

#include <bermuda.h>

#include <arch/io.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/events/event.h>

/**
 * \fn BermudaEventWait(volatile EVENT *queue, unsigned int tmo)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param tmo <i>Time out</i>. Maximum time to wait.
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
PUBLIC int BermudaEventWait(volatile EVENT *queue, unsigned int tmo)
{
        unsigned char ints = 0;
        THREAD *th;
        
        // get queue root
        BermudaSafeCli(&ints);
        th = *queue;
        BermudaIntsRestore(ints);
        
        if(th == SIGNALED)
        {
                BermudaSafeCli(&ints);
                *queue = 0;
                BermudaIntsRestore(ints);
                return 0;
        }
        
        // if the thread is not signaled
        th = BermudaCurrentThread->next;
        if(tmo)
                BermudaCurrentThread->th_timer = BermudaTimerCreate(tmo,
                                                                    &BermudaEventTMO
                                                                    BermudaCurrentThread,
                                                                    BERMUDA_ONE_SHOT);
        else
                BermudaCurrentThread->th_timer = NULL;
        
        BermudaSchedulerDisable();
        BermudaCurrentThread->flags &= ~BERMUDA_TH_STATE_MASK;
        BermudaCurrentThread->flags |= BERMUDA_THREAD_SLEEPING // mark as sleeping
        BermudaThreadQueueRemove(&BermudaThreadHead, BermudaCurrentThread);
        BermudaThreadQueueAdd((THREAD**)queue, (void*)queue);
        BermudaSchedulerResume(th); // resume scheduling
        
        /* Thread is signaled */
        if(BermudaCurrentThread->th_timer == SIGNALED)
        {
                BermudaCurrentThread->th_timer = NULL;
                return -1;
        }
        
        return 0;
}

/**
 * \fn BermudaEventTMO(VTIMER *timer, void *arg)
 * \brief Timeout function.
 * \param timer Timer object which called this function.
 * \param arg Void casted argument of the event queue, where a thread received a
 * timeout in.
 * 
 * When a thread timeouts waiting for an event, the wait function will return with
 * an error.
 */
PRIVATE WEAK void BermudaEventTMO(VTIMER *timer, void *arg)
{
        THREAD *volatile *pQueue = arg;
        
        BermudaEnterCritical();
        THREAD *thq = *pQueue;
        BermudaExitCritical();
        
        if(thq != SIGNALED)
        {
                while(thq)
                {
                        if(thq->th_timer == timer)
                        {
                                THREAD *next = thq->next;
                                BermudaThreadQueueRemove(pQueue, thq);
                                if(next == NULL)
                                {
                                        BermudaEnterCritical();
                                        *pQueue = SIGNALED;
                                        BermudaExitCritical();
                                }
                                BermudaThreadQueueAdd(&BermudaThreadHead, thq);
                        }
                        thq = thq->next;
                }
        }
}


#endif /* __EVENTS__ */
