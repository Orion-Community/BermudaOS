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

/** \file thread.c */
#if defined(__THREADS__) || defined(__DOXYGEN__)
#include <stdlib.h>
#include <string.h>

#include <arch/io.h>
#include <arch/stack.h>

#include <sys/thread.h>
#include <sys/sched.h>

/**
 * \addtogroup tmAPI Thread Management API
 * \brief The thread management module contains all functions needed to schedule
 *        threads.
 * 
 * \section algo Scheduling algorithm
 * 
 * The schedule algorithm implemented by BermudaOS, is a cooperative single queue
 * priority based algorithm.
 * 
 * \section usage Usage
 * All functions related to thread management are found in this module.\n \n
 * Example of a thread definition: \n
 * \code{.c}
 * THREAD(ExampleThread, t_arg)
 * {
 *      ExampleSetup();
 *      while(1)
 *      {
 *              BermudaThreadSleep(1000);
 *      }
 * }
 * \endcode
 * 
 * The use of BermudaThreadSleep at the end of the thread loop is essential since
 * BermudaOS implements an cooperative scheduler without a time slice. If you
 * run a thread at high priority without forcing a context switch by BermudaThreadSleep
 * or BermudaThreadWait, the system will be locked up in the thread. \n
 * The example thread can be started with this snippet:
 * \code{.c}
 * BermudaThreadCreate(th, "name" &ExampleThread, arg, stack_size, stack_p, prio);
 * \endcode
 * The first argument is an allocated pointer to an uninitialised THREAD structure.
 * 
 * @{
 */

/**
 * \brief Initialise a thread.
 * \param t Main thread
 * \param name Name of the thread.
 * \param handle Thread handle.
 * \param arg Thread argument.
 * \param stack_size Size of the stack
 * \param stack Stack pointer
 * \param prio Thead priority.
 * 
 * The given thread will be initialised, but not added to any queue. Apps won't
 * usually call this function.
 */
int BermudaThreadInit(THREAD *t, char *name, thread_handle_t handle, void *arg,
                                unsigned short stack_size, void *stack,
                                unsigned char prio)
{
        if(NULL == t)
                return -1;

        BermudaStackInit(t, stack, stack_size, handle);
        t->param = arg;
        t->prio = prio;
        t->state = THREAD_READY;
        t->name = name;
        t->sleep_time = 0;
        t->q_next = NULL;
        t->next = NULL;
        return 0;
}

/**
 * \brief Create a new thread.
 * \param t Main thread
 * \param name Name of the thread. Can be used to identify a thread later.
 * \param handle Main handle
 * \param arg Arguments to the main thread
 * \param stack_size Size of the stack
 * \param stack Stack pointer
 * \param prio Thread priority.
 * \todo Remove the <i>t</i> argument and allocate the thread in this function.
 * \todo Call a thread yield to check if the just created thread has a higher
 *       priority.
 * 
 * This function will create and start a new thread.
 */
void BermudaThreadCreate(THREAD *t, char *name, thread_handle_t handle, void *arg,
                                unsigned short stack_size, void *stack,
                                unsigned char prio)
{
        BermudaThreadInit(t, name, handle, arg, stack_size, stack, prio);
        
        // add the thread on top of the full thread list
        t->q_next = BermudaThreadHead;
        BermudaThreadHead = t;
        BermudaThreadPrioQueueAdd(&BermudaRunQueue, t);
}

/**
 * \fn BermudaThreadSleep(unsigned int ms)
 * \brief Sleep a thread.
 * \param ms Time in mili seconds to sleep.
 * \todo Not yet implemented!
 * 
 * For the given time <i>ms</i> the current thread will not be executed. When
 * ms expires the thread will be executed automaticly.
 */
void BermudaThreadSleep(unsigned int ms)
{
}

/**
 * \fn BermudaThreadTimeout(VTIMER *timer, void *arg)
 * \brief Thread sleep timeout.
 * \param timer Timer object.
 * \param arg Thread pointer.
 * 
 * This function is called by virtual timers on sleeping threads. The sleep_time
 * member will be decremented by one each run.
 */
PRIVATE WEAK void BermudaThreadTimeout(VTIMER *timer, void *arg)
{
}

/**
 * \brief Change the priority of the current thread.
 * \param prio New priority.
 * \note The scheduler will check if there are new threads which have a higher
 *       priority. If so, CPU time will be given to that thread if it is available.
 * \todo Use BermudaSchedulerExec
 * 
 * Change the priority level of the current thread.
 */
PUBLIC unsigned char BermudaThreadSetPrio(unsigned char prio)
{
        unsigned char ret = BermudaCurrentThread->prio;
        BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
        BermudaCurrentThread->prio = prio;
        if(prio < BERMUDA_LOWEST_PRIO)
                BermudaThreadPrioQueueAdd(&BermudaRunQueue, BermudaCurrentThread);
        else
                BermudaThreadExit();
        
        if(BermudaCurrentThread != BermudaRunQueue)
        {
                BermudaCurrentThread->state = THREAD_READY;
                
                BermudaEnterCritical();
                BermudaSwitchTask(BermudaRunQueue->sp);
                BermudaExitCritical();
        }
        return ret;
}

/**
 * \brief Check if there is another thread ready to run.
 * \see BermudaThreadExec
 * \note The current might resume if there is no other thread ready to schedule.
 * 
 * The scheduler will check for threads with an higher or equal priority compared
 * to the current thread. If they are available, a context switch will be done,
 * otherwise the function will return to the current thread.
 */
PUBLIC void BermudaThreadYield()
{
        if(BermudaCurrentThread->next)
        { // only do so if the current thread IS NOT the idle thread.
                BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
                BermudaThreadPrioQueueAdd(&BermudaRunQueue, BermudaCurrentThread);
        }
        
        BermudaSchedulerExec();
}

/**
 * \fn BermudaThreadWait()
 * \brief Stop the current thread.
 * \see BermudaThreadNotify()
 * 
 * The current thread will be stopped imediatly. The execution can be resumed
 * by calling BermudaThreadNotify.
 */
PUBLIC void BermudaThreadWait()
{
        BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
        BermudaCurrentThread->state = THREAD_WAITING;

        BermudaSchedulerExec();
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
PUBLIC void BermudaThreadNotify(THREAD *t)
{
        if(t != NULL)
        {
                if(t->state == THREAD_WAITING || t->state == THREAD_SLEEPING)
                { // not if the thread notifies itself or the thread the thread
                  // is already in the ready state.
                        t->state = THREAD_READY;
                        BermudaThreadPrioQueueAdd(&BermudaRunQueue, t);
                }
        }
        BermudaThreadYield();
}

/**
 * \fn BermudaThreadExit()
 * \brief Exit the current thread.
 *
 * This function will exit the given thread and delete it from the running list.
 */
PUBLIC void BermudaThreadExit()
{
        if(BermudaCurrentThread != BermudaThreadGetByName("Main Thread"))
        {
                BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
                BermudaThreadQueueRemove(&BermudaThreadHead, BermudaCurrentThread);
                BermudaThreadPrioQueueAdd(&BermudaKillQueue, BermudaCurrentThread);
        }
        BermudaThreadYield();
}

/**
 * \brief Free the threads in the kill queue.
 * \todo Delete added timers.
 * \see BermudaThreadExit
 * \see BermudaKillQueue
 * \note Applications don't call this function usually, but you can try to do so
 *       to win some memory and spare some cycles next context switch.
 * 
 * All the components of the threads in the kill queue will be released, then 
 * all threads structures will be free'd.
 */
PUBLIC void BermudaThreadFree()
{
        THREAD *kill = NULL;
        while(BermudaKillQueue)
        {
                kill = BermudaKillQueue;
                BermudaThreadQueueRemove(&BermudaKillQueue, kill);
                BermudaStackFree(kill);
                BermudaHeapFree(kill);
        }
}

/**
 * \brief Return the first thread which equals the given name.
 * \param name Name to search for.
 * \return The first thread which carries the name.
 * \note NULL is returned when there are no correspondending threads found.
 */
PUBLIC THREAD *BermudaThreadGetByName(char *name)
{
        THREAD *ret = NULL;
        
        THREAD *c = BermudaThreadHead;
        while(c)
        {
                if(!memcmp(name, c->name, strlen(c->name)))
                {
                        ret = c;
                        break;
                }
                c = c->q_next;
        }
        
        return ret;
}

/**
 * @}
 */

#endif
