/*
 *  BermudaOS - Threading module
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

/** \file sched.c */
#if (defined(__THREADS__) || defined(__DOXYGEN__))

#include <stdlib.h>
#include <bermuda.h>

#include <arch/io.h>
#include <arch/stack.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/mem.h>
#include <sys/events/event.h>

/**
 * \var BermudaCurrentThread
 * \brief Current running thread.
 * 
 * All actions on the currently running thread should be done atomicly, since
 * the scheduler can change the location where this pointer points to.
 */
THREAD *BermudaCurrentThread = NULL;

/**
 * \var BermudaThreadHead
 * \brief Scheduling queue head.
 * 
 * Root of the scheduling queue.
 */
THREAD *BermudaThreadHead = NULL;

/**
 * \var BermudaRunQueue
 * \brief List of ready to run threads.
 * 
 * Queue, sorted by priority - from high to low. The highest priority thread, and
 * thus the queue head, is always running.
 */
THREAD *BermudaRunQueue = NULL;

/**
 * \var BermudaIdleThread
 * \brief The idle thread.
 * \see IdleThread
 * 
 * This thread does exactly what it name states; idle. This threads just runs
 * an infinite loop. It will be ran when there are no other threads to run (e.g.
 * when the main thread is the only thread running and is sleeping).
 */
static THREAD *BermudaIdleThread = NULL;

/**
 * \var BermudaIdleThreadStack
 * \brief Stack for the idle thread.
 * 
 * Allocated space for the stack of the idle thread.
 */
static char BermudaIdleThreadStack[64];

/**
 * \fn IdleThread(void *arg)
 * \brief Idle thread handler.
 * 
 * When there are no other threads ready to run, the scheduler will automaticly
 * execute this thread, until another thread is ready to run.
 */
PRIVATE WEAK void IdleThread(void *arg);

/**
 * \fn BermudaSchedGetIdleThread()
 * \brief Get the idle thread.
 * \see BermudaIdleThread
 * 
 * Return the idle thread, which does nothing.
 */
inline THREAD *BermudaSchedGetIdleThread()
{
        return BermudaIdleThread;
}

/**
 * \brief Init scheduling
 * \param handle The main thread handler.
 *
 * This function will initialise the scheduler and the main thread.
 */
void BermudaSchedulerInit(thread_handle_t handle)
{
        // initialise the thread
        THREAD *t_main = BermudaHeapAlloc(sizeof(*t_main));
        BermudaThreadCreate(t_main, "Main Thread", handle, NULL, 64, NULL,
                                        100);
        
        // initialise the idle thread
        BermudaIdleThread = BermudaHeapAlloc(sizeof(*BermudaIdleThread));
        BermudaThreadCreate(BermudaIdleThread, "Idle Thread", &IdleThread, NULL, 64,
                                &BermudaIdleThreadStack[0], 255);
}

/**
 * \fn BermudaThreadAddPriQueue(THREAD * volatile *tqpp, THREAD *t)
 * \brief Add a thread to the given priority queue.
 * \param tqpp Thread Queue Pointer Pointer
 * \param t Thread to add.
 * \note The lower the priority the more important the thread is.
 * 
 * Add the given thread <i>t</i> to the priority descending queue <i>tqpp</i>. The 
 * thread will be added after the last thread with a lower priority setting.
 */
PUBLIC void BermudaThreadAddPriQueue(THREAD * volatile *tqpp, THREAD *t)
{
        THREAD *tqp;
        
        BermudaEnterCritical();
        tqp = *tqpp;
        
        if(tqp == SIGNALED)
        {
                tqp = 0;
        }
        else if(tqp)
        {
                BermudaExitCritical();
                foreach(tqp, tqp)
                {
                        if(tqp->prio > t->prio)
                                break;
                        tqpp = &tqp->next;
                }
                BermudaEnterCritical();
        }
        
        // tqp points to a thread with a lower priority then t
        t->next = tqp; // put t before tqp
        *tqpp = t; // same as prev->next = t
        
        BermudaExitCritical();
        return;
}

/**
 * \fn BermudaThreadQueueRemove(THREAD * volatile *queue, THREAD *t)
 * \brief Delete a given thread from the list.
 * \param t Thread to delete.
 * \param queue Thread queue.
 * \warning This function has not been tested yet!
 *
 * This function will delete the <i>THREAD t</i> from the linked list and fix
 * the list.
 */
PUBLIC void BermudaThreadQueueRemove(THREAD * volatile *tqpp, THREAD *t)
{
        THREAD *tqp;
        
        BermudaEnterCritical();
        tqp = *tqpp;
        BermudaExitCritical();
        
        if(tqp != SIGNALED)
        {
                foreach(tqp, tqp)
                {
                        if(tqp == t)
                        {
                                BermudaEnterCritical();
                                *tqpp = t->next;
                                BermudaExitCritical();
                                t->next = NULL;
                                break;
                        }
                        tqpp = &tqp->next;
                        continue;
                }
        }
}

/**
 * \fn BermudaThreadWait()
 * \brief Stop the current thread.
 * \return The current thread.
 * \see BermudaThreadNotify()
 * 
 * The current thread will be stopped imediatly. The execution can be resumed
 * by calling BermudaThreadNotify.
 */
THREAD *BermudaThreadWait()
{
        return NULL;
}

/**
 * \fn BermudaThreadNotify(THREAD *t)
 * \brief Notify the given thread.
 * \param t Thread to notify.
 * \note Can be called on sleeping or waiting threads. This includes threads which
 * are waiting for an event.
 * \see BermudaThreadNotify()
 * 
 * The given thread <i>t</i> will be notified and execution of the given thread
 * will be resumed.
 */
void BermudaThreadNotify(THREAD *t)
{
}

/**
 * \fn BermudaThreadExit()
 * \brief Exit the current thread.
 * \todo Make sure the task that the deleted thread is not being used anymore.
 *
 * This function will exit the given thread and delete it from the running list.
 */
void BermudaThreadExit()
{
}

/**
 * \fn BermudaSchedulerExec()
 * \brief Run the scheduler.
 * \note BermudaSchedulerExec works in the following order: \n
 *       1. Check the total thread list for posted events. If a thread has received an event,
 *          post it. \n
 *       2. Secondly, it will destroy all elapsed timers. \n
 *       3. Last, but centainly not least - it will check if a new thread has to
 *          be executed.
 * \todo Implement point 1 and 2.
 * 
 * Execute the thread with the highest priority. This might or might not be the
 * thread which is currently running.
 */
void BermudaSchedulerExec()
{
        /*
         * point 3 - execute new thread, if needed
         */
}

/**
 * \fn BermudaSchedulerGetNextRunnable(THREAD *head)
 * \brief Get the next runnable thread from the given head list.
 * \param head The thread head to check.
 * \return The next runnable thread in the list.
 * 
 * This function will search for the next runnable thread. If there aren't anymore
 * runnable threads this function returns NULL.
 */
PRIVATE WEAK THREAD *BermudaSchedulerGetNextRunnable(THREAD *head)
{
        return NULL;
}

THREAD(IdleThread, arg)
{
        while(1);
}
#endif
