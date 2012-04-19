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

#include <stdlib.h>

#include <arch/io.h>
#include <arch/stack.h>

#include <sys/sched.h>
#include <sys/thread.h>

THREAD *BermudaCurrentThread = NULL;
THREAD *BermudaPreviousThread = NULL;
THREAD *BermudaThreadHead = NULL;

void BermudaSchedulerInit(THREAD *th, thread_handle_t handle)
{
        /* initialise the thread list head */
        if(NULL == th)
                BermudaThreadHead = malloc(sizeof(*BermudaThreadHead));
        else
                BermudaThreadHead = th;
        
        BermudaThreadHead->next = NULL;
        BermudaThreadHead->prev = NULL;

        // initialise the thread
        BermudaThreadInit(BermudaThreadHead, handle, NULL, 64, NULL,
                                        BERMUDA_DEFAULT_PRIO);
        // pass control to the main thread
        BermudaSwitchTask(BermudaThreadHead->sp);
}

void BermudaSchedulerAddThread(THREAD *t)
{
}

PRIVATE WEAK void BermudaSchedulerListAdd(THREAD *th)
{
        
}
