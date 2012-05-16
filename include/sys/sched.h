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

extern THREAD *BermudaThreadHead;
extern THREAD *BermudaCurrentThread;
extern THREAD *BermudaRunQueue;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \fn BermudaSwitchTask(void *sp)
 * \brief Switch context.
 * \param sp New stack pointer.
 * \warning Interrupts should be disabled before calling this function.
 * \note Application function threads usualy don't call this function, but use
 *       BermudaThreadYield instead.
 * 
 * This function switches the thread context.
 */
extern void BermudaSwitchTask(void *sp);
extern void BermudaSchedulerInit(thread_handle_t handle);
extern void BermudaThreadQueueAddPrio(THREAD * volatile *tqpp, THREAD *t);
extern void BermudaThreadQueueRemove(THREAD * volatile *queue, THREAD *t);
extern void BermudaSchedulerExec();

PRIVATE WEAK THREAD* BermudaSchedulerGetNextRunnable(THREAD *head);

__DECL_END

#endif