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

/**
 * \def BermudaThreadYield()
 * \brief This will yield the current thread and pass control to the next one.
 * \warning This has not been tested yet!
 *
 * This define calls the function BermudaSchedulerExec, which will execute the
 * schedule algorithm immediately.
 */
#define BermudaThreadYield() BermudaSchedulerExec()

#ifdef __cplusplus
extern "C" {
#endif

extern void BermudaSchedulerInit(thread_handle_t handle);
extern void BermudaThreadAddPriQueue(THREAD * volatile *tqpp, THREAD *t);
extern void BermudaThreadQueueRemove(THREAD * volatile *queue, THREAD *t);
extern void BermudaThreadExit();
extern void BermudaSchedulerExec();
extern void BermudaThreadNotify(THREAD *t);
extern THREAD *BermudaThreadWait();

PRIVATE WEAK THREAD* BermudaSchedulerGetNextRunnable(THREAD *head);

__DECL_END

#endif