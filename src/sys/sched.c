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
#if defined(__THREADS__) || defined(__DOXYGEN__)

#include <stdlib.h>
#include <bermuda.h>

#include <arch/io.h>
#include <arch/stack.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/mem.h>

THREAD *BermudaCurrentThread = NULL;
THREAD *BermudaPreviousThread = NULL;

THREAD *BermudaThreadHead = NULL;

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

static char BermudaIdleThreadStack[64];
PRIVATE WEAK void IdleThread(void *arg);
unsigned char BermudaSchedulerEnabled = 0;

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
 * \fn BermudaSchedulerInit(THREAD *th, thread_handle_t handle)
 * \brief Init scheduling
 * \param th The main thread.
 * \param handle The main thread handler.
 *
 * This function will initialise the scheduler and the main thread.
 */
void BermudaSchedulerInit(THREAD *th, thread_handle_t handle)
{
        /* initialise the thread list head */
        if(NULL == th)
                BermudaThreadHead = BermudaHeapAlloc(sizeof(*BermudaThreadHead));
        else
                BermudaThreadHead = th;
        
        BermudaThreadHead->next = NULL;
        BermudaThreadHead->prev = NULL;

        // initialise the thread
        BermudaThreadInit(BermudaThreadHead, "Main Thread", handle, NULL, 64, NULL,
                                        BERMUDA_DEFAULT_PRIO);
        
        // initialise the idle thread
        BermudaIdleThread = BermudaHeapAlloc(sizeof(*BermudaIdleThread));
        BermudaThreadInit(BermudaIdleThread, "Idle Thread", &IdleThread, NULL, 64,
                                &BermudaIdleThreadStack[0], BERMUDA_DEFAULT_PRIO);
}

/**
 * \fn BermudaSchedulerAddThread(THREAD *head, THREAD *t)
 * \brief Add a new thread to the list
 * \param th Thread to add.
 *
 * This function will edit the thread list to add the new thread <i>th</i>.
 */
void BermudaSchedulerAddThread(THREAD *head, THREAD *t)
{
        if(NULL == t)
                return;
        
        BermudaThreadEnterIO(BermudaCurrentThread); // stop the scheduler
        THREAD *last = BermudaSchedulerGetLastThread(head);

        last->next = t;
        t->next = NULL;
        t->prev = last;
        
        BermudaThreadExitIO(BermudaCurrentThread); // continue scheduling
}

/**
 * \fn BermudaSchedulerGetLastThread(THREAD *head)
 * \brief Find the last item of BermudaThreadHead.
 * \return The last entry of the thread list.
 * This function will return the last entry of the thread list.
 */
PRIVATE WEAK THREAD* BermudaSchedulerGetLastThread(THREAD *head)
{
        THREAD *carriage = head;
        for(; carriage != NULL && carriage != carriage->next; carriage =
                                                                carriage->next)
        {
                if(carriage->next == NULL)
                        break;
        }
        // carriage points now to the last block of the thread list
        return carriage;
}

/**
 * \fn BermudaSchedulerDeleteThread(THREAD *t)
 * \brief Delete a given thread from the list.
 * \param t Thread to delete.
 * \warning This function has not been tested yet!
 *
 * This function will delete the <i>THREAD t</i> from the linked list and fix
 * the list.
 */
PRIVATE WEAK void BermudaSchedulerDeleteThread(THREAD *t)
{
        BermudaThreadEnterIO(BermudaCurrentThread);

        if(t->prev == NULL) // we're at the list head
        {
                t->next->prev = NULL;
                BermudaThreadHead = t->next;
        }
        else if(t->next == NULL) // we're at the tail of the list
        {
                t->prev->next = NULL;
        }
        else // we're somewhere in the middle of nowhere
        {
                t->prev->next = t->next;
                t->next->prev = t->next;
        }

        t->next = NULL;
        t->prev = NULL;
        
        BermudaThreadExitIO(BermudaCurrentThread);
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
        BermudaThreadEnterIO(BermudaCurrentThread);
        THREAD *ret = BermudaCurrentThread;
        BermudaThreadExitIO(BermudaCurrentThread);
        
        BermudaSchedulerDeleteThread(ret);
        
        return ret;
}

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
void BermudaThreadNotify(THREAD *t)
{
        BermudaSchedulerAddThread(BermudaThreadHead, t);
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
        THREAD *t = BermudaCurrentThread;
        if(t->next == NULL && t->prev == NULL)
                return;
        BermudaSchedulerDeleteThread(t);
        BermudaHeapFree(t->stack);
        BermudaHeapFree(t);
        BermudaSchedulerExec();
}

/**
 * \fn BermudaSchedulerExec()
 * \brief Run the scheduler.
 * 
 * This function is called from the timer interrupt (and thus runs in an interrupt)
 * it should not be called by any other part of the system.
 */
void BermudaSchedulerExec()
{
        unsigned char ints = 0;
        BermudaSafeCli(&ints);
        
        THREAD *next = NULL;
        if(BermudaCurrentThread == BermudaIdleThread)
                next = BermudaSchedulerGetNextRunnable(BermudaThreadHead);
                // start at the head when idle has ran
        else
                next = BermudaSchedulerGetNextRunnable(BermudaCurrentThread);
        
        if(NULL == next)
                next = BermudaIdleThread; // idle if there is no runnable thread
        
        if(BermudaCurrentThread == next)
        { // just return if the next thread is the same as the current one
                *(AvrIO->sreg) |= ints;
                return;
        }
                
        BermudaCurrentThread->flags &= ~BERMUDA_TH_STATE_MASK;
        if(BermudaCurrentThread->sleep_time == 0)
                BermudaCurrentThread->flags |= (THREAD_READY << 
                                                        BERMUDA_TH_STATE_BITS);
        else
        { // flag as sleeping when sleep_time != 0
                BermudaCurrentThread->flags |= (THREAD_SLEEPING << 
                                                        BERMUDA_TH_STATE_BITS);
        }
        
        next->flags &= ~BERMUDA_TH_STATE_MASK;
        next->flags |= (THREAD_RUNNING << BERMUDA_TH_STATE_BITS);

        /* do the actual swap of threads */
        BermudaPreviousThread = BermudaCurrentThread;
        BermudaCurrentThread = next;

        BermudaSwitchTask(BermudaCurrentThread->sp);
        BermudaIntsRestore(ints);
        return;
}

void BermudaSchedulerStart()
{
        BermudaCurrentThread = BermudaThreadHead;
        BermudaCurrentThread->flags &= ~BERMUDA_TH_STATE_MASK;
        BermudaCurrentThread->flags |= (THREAD_RUNNING << BERMUDA_TH_STATE_BITS);
        BermudaPreviousThread = NULL;
        BermudaSchedulerEnable();
        BermudaSwitchTask(BermudaCurrentThread->sp);
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
        BermudaThreadEnterIO(BermudaCurrentThread);
        THREAD *c = head;
        
        for(; c != NULL && c->next != c; c = c->next)
        {
                if((c->flags & BERMUDA_TH_STATE_MASK) == 2) // if ready
                        break;
                if(c->next == NULL) // if we're at the end (begin from start)
                {
                        if(head == BermudaThreadHead)
                        {
                                c = NULL;
                                goto out;
                        }
                        c = BermudaSchedulerGetNextRunnable(BermudaThreadHead);
                        if(c == NULL) // run only one recursive round
                                goto out;
                        break;
                }
                
        }
        
        out:
        BermudaThreadExitIO(BermudaCurrentThread);
        return c;
}

THREAD(IdleThread, arg)
{
        while(1);
}
#endif
