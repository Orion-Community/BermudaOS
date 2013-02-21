/*
 *  BermudaOS - Stack header
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

#ifndef __STACK_H_
#define __STACK_H_

#include <stdlib.h>

struct stack
{
	uint8_t *stack;
	uint8_t *sp;
	size_t size;
};

struct thread;
extern int stack_init(struct thread *thread, size_t stack_size, void *sp);

#endif