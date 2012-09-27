/*
 *  BermudaOS - LibC strlen
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

#include <bermuda.h>
#include <lib/string.h>

PUBLIC int strlen(const char *str)
{
	const char *s;
	
	for (s = str; *s; ++s);
	return (s - str);
}

PUBLIC int strnlen(const char *str, size_t len)
{
	size_t size = (size_t)strlen(str);
	
	if(size > len) {
		return len;
	} else {
		return size;
	}
}
