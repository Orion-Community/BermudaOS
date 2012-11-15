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

/** \file thread.h */

#ifndef __THREAD_H
#define __THREAD_H

#include <bermuda.h>
#include <arch/io.h>
#include <lib/binary.h>
#include <sys/virt_timer.h>

/**
 * \addtogroup tmAPI
 * @{
 */

/**
 * \typedef thread_handle_t
 * \brief Thread handle function type.
 */
typedef void (*thread_handle_t)(void *data);

/**
 * \def THREAD
 * \brief Thread handle definition.
 * \param fn Function name.
 * \param param Parameter name.
 * 
 * A function which serves a thread should be declared with this macro.
 */
#define THREAD(fn, param) \
static void __link __noinline fn(void *param); \
static void __link __noinline fn(void *param)

/**
 * \brief Only the definition of a thread.
 * \note Usualy used as forward declaration.
 * 
 * This define requires no implementation.
 */
#define THREAD_DEF(fn, param) \
static void __link __noinline fn(void *param);

/**
 * \def BERMUDA_DEFAULT_PRIO
 * \brief Default priority.
 */

#define BERMUDA_DEFAULT_PRIO 150
#define THREAD_DEFAULT_PRIO BERMUDA_DEFAULT_PRIO

/**
 * \def BERMUDA_HIGHEST_PRIO
 * \brief Highest priority available.
 * 
 * Threads running with BERMUDA_HIGHEST_PRIO are always ran when they are available.
 */
#define BERMUDA_HIGHEST_PRIO 0

/**
 * \def BERMUDA_LOWEST_PRIO
 * \brief Lowest available priority.
 */
#define BERMUDA_LOWEST_PRIO 255

/**
 * \def BermudaThreadEnterIO
 * \brief Enter IO safe state.
 * \param x Depricated argument.
 * \deprecated Was used by the round robin scheduler. Use BermudaEnterCritical
 *             instead.
 * \see BermudaEnterCritical
 */
#define BermudaThreadEnterIO(x) BermudaEnterCritical()

/**
 * \def BermudaThreadExitIO
 * \brief Exit IO safe state.
 * \param x Deprecated argument.
 * \deprecated Was used by the round robin scheduler. Use BermudaExitCritical
 *             instead.
 * \see BermudaExitCritical
 */
#define BermudaThreadExitIO(x)  BermudaExitCritical()

/**
 * \typedef thread_state_t
 * \brief Current running state of a thread.
 * 
 * This enumeration describes the state of a thread.
 */
typedef enum
{
        /**
         * \brief The thread is currently running.
         */
        THREAD_RUNNING,
        /**
         * \brief The thread is ready to be scheduled.
         */
        THREAD_READY,
        /**
         * \brief A thread having this state is not ready to run yet.
         */
        THREAD_SLEEPING,
        
        /**
         * \brief Waiting thread.
         * \see BermudaThreadNotify
         * 
         * A thread in the waiting state acts like a sleeping thread. The only
         * difference is that a waiting thread will not resume automaticly. It
         * must be waken up by BermudaThreadNotify.
         */
        THREAD_WAITING,
} thread_state_t;

/**
 * \struct thread
 * \brief Describes the state of a thread
 *  
 * The thread information structure.
 */
struct thread
{
        /**
         * \brief Queue pointer.
         * \see BermudaRunQueue
         * 
         * Pointer to the next entry in the list.
         */
        struct thread *next;
        
        /**
         * \brief Next pointer to total list of threads.
         * \note Should only be changed by BermudaThreadExit.
         * \see BermudaThreadHead
         * 
         * Points to the next thread in the total list of threads.
         */
        struct thread *q_next;
        
        /**
         * \brief Current queue pointer pointer.
         * \see BermudaPriQueueAdd
         */
        struct thread *volatile*queue;
        
        /**
         * \brief Name of the thread.
         * 
         * This value identifies the thread('s purpose).
         */
        char *name;
        
        /**
         * \brief Pointer to the end of the stack.
         * 
         * The end is the start, since the stack grows down.
         * <i>stack</i> - <i>stack_size</i> == <i>stack_start</i>
         */
        unsigned char *stack;            /* start of the stack */
        
        /**
         * \brief The stack pointer.
         * 
         * Pointer to the current location of the stack.
         */
        unsigned char *sp;      /* stack pointer */
        
        /**
         * \brief Size of the stack.
         */
        unsigned short stack_size;
        
        /**
         * \brief Thread parameter.
         * 
         * Parameter passed to the thread.
         */
        void *param;            /* thread parameter */
        
        /**
         * \brief Thread priority.
         * \see BERMUDA_DEFAULT_PRIO
         * 
         * This member identifies the priority of this thread.
         */
        unsigned char prio;

        /**
         * \brief Amount of time to sleep left.
         * 
         * This members tells the scheduler how much time the thread has to sleep.
         */
        unsigned int sleep_time;
        
        /**
         * \brief Sleep timer.
         * 
         * The timer which clocks this thread when it is asleep.
         */
        VTIMER *th_timer;
        
        /**
         * \brief Flag member.
         * 
         * Scheduling flags.
         */
        unsigned char state;
        
        /**
         * \brief Event counter.
         * \see BermudaSchedulerExec
         * 
         * When an event is posted to a from interrupt context, this counter
         * will be increased by one. BermudaSchedulerExec will decrease it by one
         * and handle the event.
         */
        unsigned char ec;
} __PACK__;

/**
 * \typedef THREAD
 * \brief Type definition of a thread.
 */
typedef struct thread THREAD;

#ifdef __cplusplus
extern "C" {
#endif

extern int BermudaThreadInit(THREAD *t, char *name, thread_handle_t handle, 
                             void *arg, unsigned short stack_size, void *stack,
                                unsigned char prio);


extern void BermudaThreadCreate(THREAD *t, char *name, thread_handle_t handle, void *arg,
                                unsigned short stack_size, void *stack,
                                unsigned char prio);

extern void BermudaThreadSleep(unsigned int ms);
extern unsigned char BermudaThreadSetPrio(unsigned char prio);
extern void BermudaThreadNotify(THREAD *t);
extern void BermudaThreadWait();
extern void BermudaThreadExit();
extern void BermudaThreadYield();
extern THREAD *BermudaThreadGetByName(char *name);
extern void BermudaThreadFree();

#if !defined(__EVENTS__) && defined(__THREADS__)
extern void BermudaIoWait(volatile void **tpp);
extern void BermudaIoSignal(volatile void **tpp);
#endif

// internal functions
PRIVATE WEAK void BermudaThreadTimeout(VTIMER *timer, void *arg);
__DECL_END

extern THREAD *BermudaCurrentThread;
extern THREAD *BermudaRunQueue;
extern THREAD *BermudaThreadHead;
extern THREAD *BermudaKillQueue;

/**
 * \brief Yield the current thread.
 */
#define thread_yield() BermudaThreadYield()

/**
 * @}
 */
#endif
