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

#include <string.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <sys/thread.H>


#define RUNNABLE(t) (((t)->flags & (AVRTH_RUNNABLE|AVRTH_VALID)) == (AVRTH_RUNNABLE|AVRTH_VALID))


THREAD  * prev_thread;
THREAD  * current_thread;
uint8_t   n_threads;
uint8_t   thread_preemption;


static THREAD * thread_sleep_q;
static THREAD * runq;
static THREAD * idle_thread;
static THREAD * main_thread;


static void put_runq(THREAD * t);

static THREAD * get_runq(void);

static void take_runq(THREAD * t);

void thread_swtch(uint8_t * sp);



/*
 * put a thread to sleep on the sleep queue
 */
void thread_sleep(uint16_t ms)
{
  THREAD * t, * p;
  uint8_t ints;

  ints = SREG & 0x80;
  cli();

  current_thread->flags &= ~AVRTH_RUNNABLE;

  if (thread_sleep_q == NULL) {
    thread_sleep_q               = current_thread;
    current_thread->sleep_q_next = NULL;
    current_thread->sleep_timer  = ms;
    thread_yield();
    SREG |= ints;
    return;
  }

  p = NULL;
  t = thread_sleep_q;
  while (t) {
    if (ms < t->sleep_timer) {
      t->sleep_timer -= ms;
      current_thread->sleep_q_next = t;
      current_thread->sleep_timer  = ms;
      if (p) {
        p->sleep_q_next = current_thread;
      }
      else {
        thread_sleep_q = current_thread;
      }
      thread_yield();
      SREG |= ints;
      return;
    }

    ms -= t->sleep_timer;
    p   = t;
    t   = t->sleep_q_next;
  }

  p->sleep_q_next              = current_thread;
  current_thread->sleep_q_next = NULL;
  current_thread->sleep_timer  = ms;
  thread_yield();
  SREG |= ints;
  return;
}


/*
 * thread_tick() - scheduling timer expired.  This is called from the
 * main application's scheduling timer interrtupt or whatever periodic
 * time source is being use to for thread timing an preemption.  We
 * assume that interrupts are disabled here, so be careful not to
 * reenable.
 *
 * Register a time quanta tick against the running thread and preempt
 * it if it's time for another to run.  Also, check the thread sleep
 * queue for any sleeping threads and decrement its sleep timer if so.
 * If the sleep timer expires, put the thread back into the run queue.  
 */
void thread_tick(void)
{
  uint8_t should_yield;
  THREAD * t;

  should_yield = 0;
  if (thread_sleep_q) {
    thread_sleep_q->sleep_timer--;
    if (thread_sleep_q->sleep_timer == 0) {
      while (thread_sleep_q && (thread_sleep_q->sleep_timer == 0)) {
        thread_sleep_q->flags |= AVRTH_RUNNABLE;
        put_runq(thread_sleep_q);
        if (thread_sleep_q->priority > current_thread->priority)
          should_yield = 1;
        t = thread_sleep_q;
        thread_sleep_q = thread_sleep_q->sleep_q_next;
        t->sleep_q_next = NULL;
      }
    }
  }

  if (should_yield) {
    thread_yield();
  }
  else {
    if (!thread_preemption)
      return;

    current_thread->ticks--;
    if (current_thread->ticks == 0) {
      thread_yield();
    }
  }
}


/*
 * idle thread entry point - lowest priority thread that runs when no
 * other threads are runnable.  This keeps from having to check the
 * special case of no runnable threads within thread_yield().  
 */
static void idle_entry(void)
{
  while(1) {
//    thread_tick(); /* uncomment if running in a simulator */
    thread_yield();
  }
}


/*
 * initialize a thread structure
 */
void thread_init(THREAD * t, void (*entry)(void), uint8_t * stack, 
                 uint16_t stack_size, uint8_t priority, void * parm)
{
  uint8_t i;

  if (stack == NULL) {
    /*
     * the stack for the 'main' thread, the thread associated with the
     * 'main()' routine, is normally passed in as NULL so that we can
     * create it just under the stack given us by the C run-time
     * initialization code.  
     */
    stack = (uint8_t *)(SP - stack_size - 2);
  }

  /*
   * initialize the thread stack
   */
  t->stack      = stack;
  t->stack_size = stack_size;
  memset(t->stack, 0x99, stack_size);
  t->sp         = &t->stack[stack_size-1];
  *t->sp--      = ((uint16_t)entry) & 0x0ff;
  *t->sp--      = (((uint16_t)entry) >> 8) & 0xff;
  for (i=0; i<33; i++) {
    if (i == 1) {
      *t->sp-- = SREG;
    }
    else {
      *t->sp-- = 0;
    }
  }

  /*
   * initialize priority, time quantum, and flags
   */
  t->priority   = priority;
  t->quantum    = MAX_QUANTUM;
  t->ticks      = MAX_QUANTUM;
  t->flags      = AVRTH_VALID | AVRTH_RUNNABLE;
  t->parm       = parm;

  return;
}

/*
 * Initialize the threads package - this initializes all data
 * structures, the idle thread, and the main thread.  We briefly
 * switch to the idle thread and back to the main thread to initialize
 * the stacks.
 */
THREAD * threads_init(uint8_t max_threads, uint16_t mstacksz, uint16_t istacksz, 
                      uint8_t main_prio, uint8_t preemption)
{
  uint8_t i;
  uint8_t ints;

  ints = SREG & 0x80;
  cli();

  /*
   * set n_threads
   */
  n_threads = max_threads;

  /*
   * initialize all threads as "not valid"
   */
  for (i=0; i<n_threads; i++) {
    thread[i].flags = 0;
  }

  runq = NULL;

  /*
   * initialize the idle thread which runs if no other threads are
   * runnable
   */
  idle_thread = thread_create(idle_entry, idle_stack, istacksz, 
                              0, NULL);
  take_runq(idle_thread);

  thread_preemption = preemption;
  thread_sleep_q    = NULL;

  /*
   * Set up main thread to use the current stack. While this
   * initializes the main_thread stack, that initialization is
   * effectively discarded, and when we switch away from this thread
   * below, the context will be saved and the stack pointer will be
   * saved correctly from within thread_swtch().  
   */
  main_thread = thread_create(NULL, NULL, mstacksz, main_prio, NULL);

  prev_thread    = main_thread;
  current_thread = idle_thread;

  /*
   * switch away from main_thread and to idle_thread, which immediately
   * calls thread_yield() resulting in a switch back to the
   * main_thread.
   */
  thread_swtch(current_thread->sp);

  SREG |= ints;

  return main_thread;
}


/*
 * create a new thread - return NULL if no more thread slots are
 * available 
 */
THREAD * thread_create(void (*entry)(void), uint8_t * stack, 
                       uint16_t stack_size, uint8_t priority, void * parm)
{
  uint8_t i;
  THREAD * t;
  uint8_t ints;

  ints = SREG & 0x80;
  cli();

  /*
   * find a free thread slot
   */
  for (t=thread, i=0; i<n_threads; i++,t++) {
    if (t->flags == 0) {
      t->flags = AVRTH_VALID;
      break;
    }
  }

  if (i == n_threads) {
    /*
     * no more thread slots available
     */
    SREG |= ints;
    return NULL;
  }

  thread_init(t, entry, stack, stack_size, priority, parm);

  /*
   * put the new thread in the run q in priority order
   */
  put_runq(t);
  if (t->priority > current_thread->priority)
    thread_yield();

  SREG |= ints;
  return t;
}


/*
 * remove a thread from the run queue.
 */
static void take_runq(THREAD * t)
{
  THREAD * r;

  r = runq;
  while (r) {
    if (r == t) {
      if (r->runq_prev) {
        r->runq_prev->runq_next = r->runq_next;
      }
      if (r->runq_next) {
        r->runq_next->runq_prev = r->runq_prev;
      }
      if (runq == r) {
        runq = r->runq_next;
      }
      r->runq_next = NULL;
      r->runq_prev = NULL;
      return;
    }
    r = r->runq_next;
  }
}


/*
 * remove a thread from the sleep queue.
 */
static void take_sleepq(THREAD * t)
{
  THREAD * r, * p;

  p = NULL;
  r = thread_sleep_q;
  while (r) {
    if (r == t) {
      if (t->sleep_q_next) {
        t->sleep_q_next->sleep_timer += t->sleep_timer;
      }
      if (thread_sleep_q == t) {
        thread_sleep_q = t->sleep_q_next;
      }
      if (p) {
        p->sleep_q_next = r->sleep_q_next;
      }
      return;
    }
    p = r;
    r = r->sleep_q_next;
  }
}


/*
 * kill a thread - remove it from the run queue and mark its slot
 * available for use.  
 */
void thread_kill(THREAD * t)
{
  uint8_t ints;

  ints = SREG & 0x80;
  cli();

  take_runq(t);
  take_sleepq(t);
  t->flags = 0;

  if (current_thread == t) {
    thread_yield();
  }

  SREG |= ints;
}


/*
 * stop a thread - remove it from the run queue.  The thread can be
 * restarted with 'thread_start()' 
 */
void thread_stop(THREAD * t)
{
  uint8_t ints;

  ints = SREG & 0x80;
  cli();

  take_runq(t);
  take_sleepq(t);
  t->flags &= ~AVRTH_RUNNABLE;

  if (current_thread == t) {
    thread_yield();
  }

  SREG |= ints;
}


/*
 * restart a stopped thread
 */
void thread_start(THREAD * t)
{
  uint8_t ints;

  if (t == current_thread)
    return;

  ints = SREG & 0x80;
  cli();

  take_runq(t);
  take_sleepq(t);

  t->flags |= AVRTH_RUNNABLE;
  put_runq(t);
  if (t->priority > current_thread->priority)
    thread_yield();

  SREG |= ints;
}


/*
 * place a thread on the run queue in priority order
 */
static void put_runq(THREAD * t)
{
  THREAD * r;

  r = runq;
  while (r) {
    if (t->priority > r->priority) {
      if (r->runq_prev) {
        t->runq_prev = r->runq_prev;
        r->runq_prev->runq_next = t;
        r->runq_prev = t;
        t->runq_next = r;
      }
      else {
        runq = t;
        t->runq_next = r;
        t->runq_prev = NULL;
        t->runq_next->runq_prev = t;
      }
      return;
    }

    if (r->runq_next == NULL) {
      /*
       * we've reached the end of the run q, put the thread on the end 
       */
      r->runq_next = t;
      t->runq_next = NULL;
      t->runq_prev = r;
      return;
    }

    r = r->runq_next;
  }

  /*
   * run q is empty
   */
  runq = t;
  t->runq_next = NULL;
  t->runq_prev = NULL;
}


/*
 * retrieve the highest priority thread from the run queue
 */
static THREAD * get_runq(void)
{
  THREAD * r;

  if (runq) {
    r = runq;
    runq = r->runq_next;
    if (runq) {
      runq->runq_prev = NULL;
    }
    r->runq_next = NULL;
    r->runq_prev = NULL;
    return r;
  }

  return NULL;
}


/*
 * thread_yield() - allows the current thread to give up control of
 * the CPU allowing another to run.  Use a simple round-robin
 * scheduler.  If no other threads of an equal or higher priority are
 * runnable, no thread switch actually occurs.
 */
void thread_yield(void)
{
  uint8_t ints;
  THREAD * t;

  ints = SREG & 0x80;
  cli();

  if (RUNNABLE(current_thread)) {
    /*
     * put ourselves back into the runq
     */
    put_runq(current_thread);
  }

  /*
   * select the highest priority runnable thread to run
   */
  t = get_runq();

  if (current_thread == t) {
    current_thread->ticks = current_thread->quantum;
    SREG |= ints;
    return;
  }
  else {
    prev_thread           = current_thread;
    current_thread        = t;
    prev_thread->ticks    = prev_thread->quantum;
    current_thread->ticks = current_thread->quantum;
    
    thread_swtch(current_thread->sp);
  }

  SREG |= ints;
  return;
}


/*
 * this is called from within the assembly language routine
 * 'thread_swtch()' in order to save the previous thread's stack
 * pointer value.  
 */
void thread_save_sp(uint8_t * sp)
{
  sp = sp + 2;
  prev_thread->sp = sp;
}
