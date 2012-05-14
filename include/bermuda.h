/*
 *  BermudaOS - BermudaOS stdlib
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
 * \file bermuda.h 
 * \brief BermudaOS standard library header.
 * 
 * This header contains all basic defines, functions and types.
 */

#ifndef __BERMUDA_H
#define __BERMUDA_H

#ifdef __cplusplus
#include <cplusplus.h>

#define __DECL extern "C" {
#define __DECL_END }

#else

#define __DECL
#define __DECL_END

#endif

#define __PACK__ __attribute__((packed))

/**
 * \def PRIVATE
 * \brief Hidden visibility.
 * 
 * The function is not visible outside of its own compile unit.
 */
#define PRIVATE __attribute__ ((visibility ("hidden")))

/**
 * \def WEAK
 * \brief Declare a function weak.
 * 
 * Weak functions are not marked as global functions. Same as <i>'static'</i>.
 */
#define WEAK    __attribute__((weak))

/**
 * \def PUBLIC
 * \brief Declare a function public.
 * 
 * Function declared public are explicitly marked visible, so other compile units
 * can access the function.
 */
#define PUBLIC  __attribute__((externally_visible))

/**
 * \def __raw
 * \brief Raw function.
 * 
 * Raw function have no epilogue or prologue. The programmer has to provide them.
 * This can be useful in ISR's.
 */
#define __raw   __attribute__((naked))

/**
 * \def signal
 * \brief ISR
 * 
 * Declare a function as an ISR.
 */
#define __sig   __attribute__((signal))

/**
 * \typedef mutex_t
 * \brief Mutual exclusion type.
 */
typedef unsigned char mutex_t;

#include <sys/mem.h>

#endif
