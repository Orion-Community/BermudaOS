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

#define PRIVATE __attribute__ ((visibility ("hidden")))
#define WEAK    __attribute__((weak))
#define __raw   __attribute__((naked))
#define __sig   __attribute__((signal))

typedef unsigned char mutex_t;

#include <sys/mem.h>

#endif
