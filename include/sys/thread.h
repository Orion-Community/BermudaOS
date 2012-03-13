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

#ifndef __THREAD_H
#define __THREAD_H

#include <inttypes.h>

/*
 * max sched timer ticks before preempting the running thread
 */
#define MAX_QUANTUM 5

typedef struct thread_t {
  struct thread_t * runq_prev;    /* previous thread in run queue */
  struct thread_t * runq_next;    /* next thread in run queue */
  struct thread_t * sleep_q_next; /* next thread in sleep queue */
  uint8_t         * sp;           /* thread stack pointer */
  uint8_t         * stack;        /* start of thread stack */
  uint8_t           priority;     /* thread priority */
  uint8_t           ticks;        /* sched timer ticks remaining */
  uint8_t           quantum;      /* sched timer ticks allowed */
  uint8_t           flags;        /* thread flags - see AVRTH_* flags below */
  uint16_t          stack_size;   /* thread stack size */
  uint16_t          sleep_timer;  /* sleep timer */
  void            * parm;         /* parameter pointer */
} THREAD;

#define AVRTH_VALID    0x01  /* thread is valid (in use) */
#define AVRTH_RUNNABLE 0x02  /* thread is runnable */


#define THREADS(n, istacksz) \
  THREAD thread[n+2]; \
  THREAD * main_thread; \
  THREAD_DEF(idle, istacksz)

#define THREAD_DEF(name, stacksz) \
  uint8_t name##_stack[stacksz]; \
  THREAD * name##_thread

#define N_THREADS  (sizeof(thread)/sizeof(THREAD))

/*
 * User code needs to declare these arrays, sized accordingly
 */
extern THREAD  thread[];             /* max # threads allowed */
extern uint8_t idle_stack[];         /* storage for idle thread stack */

/*
 * these are declared within the threads code
 */
extern THREAD * current_thread;
extern uint8_t n_threads;
extern uint8_t thread_preemption;


void thread_tick(void);

THREAD * threads_init(uint8_t max_threads, uint16_t mstacksz, uint16_t istacksz, 
                      uint8_t main_prio, uint8_t preemption);

void thread_yield(void);

THREAD * thread_create(void (*entry)(void), uint8_t * stack, 
                       uint16_t stack_size, uint8_t priority, void * parm);

void thread_sleep(uint16_t ms);

void thread_start(THREAD * t);

void thread_stop(THREAD * t);

void thread_kill(THREAD * t);

#endif
