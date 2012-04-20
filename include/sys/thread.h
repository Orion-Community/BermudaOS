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

#include <lib/binary.h>

#include <inttypes.h>
#include <avr/io.h>

typedef void (*thread_handle_t)(void *data);

#define THREAD(fn, param) \
PRIVATE WEAK void fn(void *param); \
PRIVATE WEAK void fn(void *param)

#define BermudaThreadDoesIO(th)  (th->flags & B1)
#define BermudaThreadEnterIO(th) (th->flags |= B1);
#define BermudaThreadExitIO(th)  (th->flags &= (~B1))

#ifndef RTSCHED
#define BERMUDA_DEFAULT_PRIO 150

/**
  * \struct struct thread
  * \brief Describes the state of a thread
  */
struct thread
{
        struct thread *next;
        struct thread *prev;
        unsigned int id;
        char *name;
        unsigned char *stack;            /* start of the stack */
        unsigned char *sp;      /* stack pointer */
        void *param;            /* thread parameter */
        unsigned char prio;
        unsigned short stack_size;
        unsigned int sleep_time;
        unsigned char flags; /*
                              * [0] If one, stop scheduling, IO is performing
                              * [1-7] reserved
                              */
} __attribute__((packed));
typedef struct thread THREAD;

__DECL
extern int BermudaThreadInit(THREAD *t, thread_handle_t handle, void *arg,
                                unsigned short stack_size, void *stack,
                                unsigned char prio);
extern void BermudaSwitchTask(void *sp);

PRIVATE WEAK int *BermudaThreadNativeCreate();
__DECL_END

extern THREAD *BermudaCurrentThread;
extern THREAD *BermudaPreviousThread;

#endif

#endif
