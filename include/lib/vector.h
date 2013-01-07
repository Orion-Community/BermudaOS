/*
 *  BermudaOS - Vector header
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

#ifndef __VECTOR_H
#define __VECTOR_H

#define vector_foreach(__vec, __it, __ptr, __ptr_type) \
size_t __it = 0; \
typeof(__ptr_type) *__ptr; \
for(__ptr = (__vec)->data[0]; __it < (__vec)->length; ++(__it), __ptr = (__vec)->data[__it])

#define VECTOR(__symbol, __datatype)\
struct __symbol \
{ \
	size_t length, limit; \
	typeof(__datatype) *volatile*data; \
};
#endif