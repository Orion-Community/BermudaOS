/*
 *  BermudaOS - Schedule module
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

/** \file sched.h */
#ifndef __SCHED_H_
#define __SCHED_H_

#include <bermuda.h>
#include <sys/thread.h>

extern unsigned char BermudaSchedulerEnabled;
extern THREAD *BermudaThreadHead;

__DECL
/**
 * \fn BermudaThreadInit(THREAD *t, thread_handle_t handle, void *arg, unsigned short stack_size, void *stack)
 * \brief Initialize the scheduler with main thread.
 * \param t Main thread
 * \param handle Main handle
 * \param arg Arguments to the main thread
 * \param stack_size Size of the stack
 * \param stack Stack pointer
 *
 * Initialize the main thread. If <i>stack</i> is NULL, then the current stack
 * pointer will be used.
 */
extern void BermudaSchedulerInit(THREAD *th, thread_handle_t handle);

/**
 * \fn BermudaSchedulerAddThread(THREAD *head, THREAD *t)
 * \brief Add a new thread to the list
 * \param th Thread to add.
 *
 * This function will edit the thread list to add the new thread <i>th</i>.
 */
extern void BermudaSchedulerAddThread(THREAD *head, THREAD *th);

/**
 * \fn BermudaSchedulerGetLastThread(THREAD *head)
 * \brief Find the last item of BermudaThreadHead.
 * \return The last entry of the thread list.
 * This function will return the last entry of the thread list.
 */
PRIVATE WEAK THREAD* BermudaSchedulerGetLastThread(THREAD *head);

/**
 * \fn BermudaSchedulerDeleteThread(THREAD *t)
 * \brief Delete a given thread from the list.
 * \param t Thread to delete.
 *
 * This function will delete the <i>THREAD t</i> from the linked list and fix
 * the list.
 */
PRIVATE WEAK void BermudaSchedulerDeleteThread(THREAD *t);

/**
 * \fn BermudaThreadExit()
 * \brief Exit the current thread.
 * \todo Make sure the task that the deleted thread is not being used anymore.
 *
 * This function will exit the given thread and delete it from the running list.
 */
extern void BermudaThreadExit();

/**
 * \fn BermudaSchedulerStart()
 * \brief Start the scheduler.
 *
 * This function will enable the scheduler and pass control to the thread head.
 */
extern void BermudaSchedulerStart();

/**
 * \fn BermudaSchedulerGetNextRunnable(TRHEAD *head)
 * \brief Get the next runnable thread from the given head list.
 * \param head The thread head to check.
 * \return The next runnable thread in the list.
 * 
 * This function will search for the next runnable thread. If there aren't anymore
 * runnable threads this function returns NULL.
 */
PRIVATE WEAK THREAD* BermudaSchedulerGetNextRunnable(THREAD *head);

/**
 * \fn BermudaSchedulerExec()
 * \brief Run the scheduler.
 * 
 * This function is called from the timer interrupt (and thus runs in an interrupt)
 * it should not be called by any other part of the system.
 */
extern void BermudaSchedulerExec();

/**
 * \fn BermudaThreadNotify(THREAD *t)
 * \brief Notify the given thread.
 * \param t Thread to notify.
 * \warning Do not call this function is the thread is not in a waiting state.
 * \note Waiting state is NOT sleeping.
 * \see BermudaThreadNotify()
 * 
 * The given thread <i>t</i> will be notified and execution of the given thread
 * will be resumed.
 */
extern void BermudaThreadNotify(THREAD *t);

/**
 * \fn BermudaThreadWait()
 * \brief Stop the current thread.
 * \return The current thread.
 * \see BermudaThreadNotify()
 * 
 * The current thread will be stopped imediatly. The execution can be resumed
 * by calling BermudaThreadNotify.
 */
extern THREAD *BermudaThreadWait();

/**
 * \fn BermudaSchedulerDisable()
 * \brief This function will disable.
 *
 * This function will stop the scheduler and pass control to the main thread.
 */
static inline void BermudaSchedulerDisable()
{
        BermudaSchedulerEnabled = 0;
}

/**
 * \fn BermudaSchedulerEnable()
 * \brief Enable the scheduler.
 * 
 * This function will start the scheduler
 */
static inline void BermudaSchedulerEnable()
{
        BermudaSchedulerEnabled = 1;
}

/**
 * \fn BermudaSchedGetIdleThread()
 * \brief Get the idle thread.
 * \see IdleThread
 * 
 * Return the idle thread, which does nothing.
 */
extern inline THREAD *BermudaSchedGetIdleThread();

__DECL_END

/**
 * \def BermudaThreadYield()
 * \brief This will yield the current thread and pass control to the next one.
 * \warning This has not been tested yet!
 *
 * This define calls the function BermudaSchedulerExec, which will execute the
 * schedule algorithm immediately.
 */
#define BermudaThreadYield() BermudaSchedulerExec()

#endif