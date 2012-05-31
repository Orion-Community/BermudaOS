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
 * \addtogroup event_management Event Management API
 * \brief Thread synchronization principles.
 * 
 * A thread may wait for a certain event by using the function BermudaEventWait
 * . Another thread can wakeup the event by using the function BermudaEventSignal. \n
 * 
 * The function BermudaEventPost may <b>NEVER</b> used from interrupt context. When
 * an event has to be posted from interrupt context, one can use the function
 * BermudaEventSignalFromISR. \n
 * \n
 * If an event is posted, the signaled thread takes over the CPU (if its priority
 * is higher or equal to the one running).\n
 * \n
 * An event queue needs the following definition:
 * \code{.c}
 * volatile THREAD *QueueName;
 * \endcode
 * 
 * @{
 */

/**
 * \fn BermudaEventWait(volatile THREAD **tqpp, unsigned int tmo)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param tmo <i>Time out</i>. Maximum time to wait.
 * \see BermudaEventSignal
 * \see BermudaEventSignalFromISR
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
PUBLIC int BermudaEventWait(volatile THREAD **tqpp, unsigned int tmo)
{
	volatile THREAD *tqp;

	BermudaEnterCritical();
	tqp = *tqpp;
	BermudaExitCritical();
        
	if(tqp == SIGNALED) { // queue is empty
		BermudaEnterCritical();
		*tqpp = 0;
		BermudaExitCritical();
                
		BermudaThreadYield(); // give other threads a chance
		return 0;
	}
        
	/*
	 * The queue is currently locked, this means the current thread has
	 * to wait. Remove it from the running queue and add it to the event
	 * queue.
	 */
	BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
	BermudaThreadPrioQueueAdd((THREAD**)tqpp, BermudaCurrentThread);
	BermudaCurrentThread->state = THREAD_SLEEPING;
        
	if(tmo) {
		BermudaCurrentThread->th_timer = BermudaTimerCreate(tmo, &BermudaEventTMO,
		                                                   (void*)tqpp,
		                                                   BERMUDA_ONE_SHOT
		);
	}
	else {
		BermudaCurrentThread->th_timer = NULL;
	}
	BermudaSchedulerExec(); // DO NOT USE THREAD YIELDING! -> queue's are editted
        
	// When the thread returns
	if(BermudaCurrentThread->th_timer == SIGNALED) { // event timed out
		BermudaCurrentThread->th_timer = NULL;
		return -1;
	}
        
	return 0; // event posted succesfuly
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
	THREAD *volatile *tqpp, *tqp;
        
	tqpp = (THREAD**)arg;
	BermudaEnterCritical();
	tqp = *tqpp;
	BermudaExitCritical();
        
	if(tqp != SIGNALED) {
		while(tqp) {
			if(tqp->th_timer == timer) { 
			// found the timed out thread
				BermudaEnterCritical();
				*tqpp = tqp->next;
				if(tqp->ec) {
					if(tqp->next) {
						tqp->next->ec = tqp->ec;
					}
					else {
						*tqpp = SIGNALED;
					}
					tqp->ec = 0;
				}
				BermudaExitCritical();
                                
				// mark thread as ready to go
				tqp->state = THREAD_READY;
				BermudaThreadPrioQueueAdd(&BermudaRunQueue, tqp);
				tqp->th_timer = SIGNALED; // waiting thread can handle
										  // on this signal
				break;
            }
			tqpp = &tqp->next;
			tqp = tqp->next;
		}
	}
}

/**
 * \brief Post an event.
 * \param tqpp Queue to post an event to.
 * \see BermudaEventWait
 * 
 * The given queue will be signaled and the thread with the highest priority will
 * become runnable again.
 */
PUBLIC int BermudaEventSignalRaw(THREAD *volatile*tqpp)
{
	THREAD *t;
	BermudaEnterCritical();
	t = *tqpp;
	BermudaExitCritical();
        
	if(t != SIGNALED) {
		if(t) {
			BermudaEnterCritical();
			*tqpp = t->next;
			if(t->ec) {
				if(t->next)
					t->next->ec = t->ec;
				else
					*tqpp = SIGNALED;

				t->ec = 0;
			}
			BermudaExitCritical();
                        
			if(t->th_timer) {
				BermudaTimerStop(t->th_timer);
				t->th_timer = NULL;
			}
                        
			t->state = THREAD_READY;
			BermudaThreadPrioQueueAdd(&BermudaRunQueue, t);
                        
			return 0; // post done succesfuly
		}
		else {
			BermudaEnterCritical();
			*tqpp = SIGNALED;
			BermudaExitCritical();
		}
    }
	return 1; // could not post
}

/**
 * \brief Post an event.
 * \param tqpp Queue to post an event to.
 * \see BermudaEventWait
 * 
 * The given queue will be signaled and the thread with the highest priority will
 * become runnable again. The signaled thread will run if it has an equal or
 * higher priority than the currently running thread.
 */
PUBLIC int BermudaEventSignal(volatile THREAD **tqpp)
{
        int ret = BermudaEventSignalRaw((THREAD*volatile*)tqpp);
        BermudaThreadYield();
        return ret;
}

/**
 * \brief Post an event from an ISR.
 * \param tqpp Event queue.
 * \see BermudaEventSignal
 * 
 * Should only be used when an event has to be posted from an ISR. If you are in
 * a normal context, BermudaEventSignal should be used.
 */
PUBLIC void BermudaEventSignalFromISR(volatile THREAD **tqpp)
{
	if(*tqpp == NULL) {
		*tqpp = SIGNALED;
	}
	else if(*tqpp != SIGNALED) {
		(*tqpp)->ec++;
	}
}

// @}
#endif /* __EVENTS__ */
