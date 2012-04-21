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
void BermudaSchedulerInit(THREAD *th, thread_handle_t handle);

/**
 * \fn BermudaSchedulerAddThread(THREAD *t)
 * \brief Add a new thread to the list
 * \param th Thread to add.
 *
 * This function will edit the thread list to add the new thread <i>th</i>.
 */
void BermudaSchedulerAddThread(THREAD *t);

/**
 * \fn BermudaSchedulerGetLastThread()
 * \brief Find the last item of BermudaThreadHead.
 * \return The last entry of the thread list.
 * This function will return the last entry of the thread list.
 */
PRIVATE WEAK THREAD* BermudaSchedulerGetLastThread();

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
 * \fn BermudaThreadExit(THREAD *t)
 * \brief Exit the given thread.
 * \param t Thread to exit.
 *
 * This function will exit the given thread and delete it from the running list.
 */
void BermudaThreadExit(THREAD *t);

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
void BermudaSchedulerExec();
__DECL_END
#endif