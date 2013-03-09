/*
 *  BermudaOS - StdInt header
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

#ifndef __STDINT_H_
#define __STDINT_H_

#include <arch/types.h>

/**
 * \typedef mutex_t
 * \brief Mutual exclusion type.
 */
typedef volatile unsigned char mutex_t;

typedef volatile unsigned char*   reg8_t; //!< 8-bit register type.
typedef volatile unsigned short*  reg16_t; //!< 16-bit register type.
typedef volatile uint32_t*        reg32_t; //!< 32-bit register type.
typedef unsigned char             bool; //!< Boolean type definition.
typedef uint64_t                  sys_tick_t; //!< System tick type definition.

#endif /* __STDINT_H_ */

