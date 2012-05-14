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

#include <lib/binary.h>
#include <sys/virt_timer.h>

typedef void (*thread_handle_t)(void *data);

#define THREAD(fn, param) \
PRIVATE WEAK void fn(void *param); \
PRIVATE WEAK void fn(void *param)

#define BERMUDA_TH_STATE_BITS 1
#define BERMUDA_TH_STATE_MASK (B11 << BERMUDA_TH_STATE_BITS)
#define BERMUDA_TH_IO_MASK 0x1

#define BermudaThreadDoesIO(th)  (th->flags & BERMUDA_TH_IO_MASK)
#define BermudaThreadEnterIO(th) (th->flags |= BERMUDA_TH_IO_MASK)
#define BermudaThreadExitIO(th)  (th->flags &= (~BERMUDA_TH_IO_MASK))

#ifndef RTSCHED
#define BERMUDA_DEFAULT_PRIO 150

/**
 * \typedef thread_state_t
 * \brief Current running state of a thread.
 * 
 * This enumeration describes the state of a thread.
 */
typedef enum
{
        /**
         * \enum RUNNING
         * \brief The thread is currently running.
         */
        THREAD_RUNNING,
        /**
         * \enum READY
         * \brief The thread is ready to be scheduled.
         */
        THREAD_READY,
        /**
         * \enum SLEEPING
         * \brief A thread having this state is not ready to run yet.
         */
        THREAD_SLEEPING,
} thread_state_t;

/**
  * \struct struct thread
  * \brief Describes the state of a thread
  */
struct thread
{
        /**
         * \brief Next pointer.
         */
        struct thread *next;
        /**
         * \brief Previous pointer
         */
        struct thread *prev;
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
        unsigned char flags; /*
                              * [0] If one, stop scheduling, IO is performing
                              * [1-2] thread_state_t
                              */
} __attribute__((packed));
typedef struct thread THREAD;

__DECL
extern int BermudaThreadInit(THREAD *t, char *name, thread_handle_t handle, 
                             void *arg, unsigned short stack_size, void *stack,
                                unsigned char prio);
extern void BermudaSwitchTask(void *sp);
extern void BermudaThreadSleep(unsigned int ms);

__DECL_END

extern THREAD *BermudaCurrentThread;
extern THREAD *BermudaPreviousThread;

#endif

#endif
