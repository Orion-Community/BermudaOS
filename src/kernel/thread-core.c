/*
 *  BermudaOS - Thread core
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
#include <stdio.h>

#include <kernel/thread.h>
#include <kernel/stack.h>
#include <kernel/ostimer.h>

/**
 * \brief Currently running threads.
 * 
 * Each CPU core has its own running thread.
 */
static struct thread *current_thread[CPU_CORES];

/**
 * \brief The idle thread definition.
 */
static struct thread idle_thread;
/**
 * \brief The main thread definition.
 */
static struct thread main_thread;
