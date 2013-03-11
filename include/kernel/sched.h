/*
 *  BermudaOS - Thread private header.
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

/**
 * \file include/kernel/sched.h Scheduler header.
 */

#ifndef __SCHED_H
#define __SCHED_H

#ifndef SYS_TICK_RATE
/**
 * \brief System tick rate.
 *
 * SYS_TICK_RATE defines the speed of the system tick in miliseconds
 */
#define SYS_TICK_RATE 1
#endif /* SYS_TICK_RATE */

#ifndef SCHED_TICK_RATE
/**
 * \brief SCHED_TICK_RATE defines the scheduler time slice in miliseconds.
 */
#define SCHED_TICK_RATE 1
#endif /* SCHED_TICK_RATE */

#endif /* __SCHED_H */

