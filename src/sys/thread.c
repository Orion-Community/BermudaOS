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

#include <arch/io.h>
#include <arch/stack.h>

#include <sys/thread.h>
#include <sys/sched.h>

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
        BermudaThreadAddPriQueue(&BermudaRunQueue, t);
}

/**
 * \fn BermudaThreadSleep(unsigned int ms)
 * \brief Sleep a thread.
 * \param ms Time in mili seconds to sleep.
 * 
 * For the given time <i>ms</i> the current thread will not be executed. When
 * ms expires the thread will be executed automaticly.
 */
void BermudaThreadSleep(unsigned int ms)
{
        unsigned char ints = 0;
        BermudaSafeCli(&ints);
        
        BermudaCurrentThread->sleep_time = ms;
        BermudaCurrentThread->th_timer = BermudaTimerCreate(1, &BermudaThreadTimeout,
                                                            BermudaCurrentThread,
                                                            BERMUDA_PERIODIC);
        BermudaSchedulerExec();
        
        BermudaIntsRestore(ints);
        return;
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
        THREAD *t = (THREAD*)arg;
        t->sleep_time--;
        
        if(t->sleep_time == 0)
        {
                t->state = THREAD_READY;
                BermudaTimerDelete(timer);
        }
}
#endif
