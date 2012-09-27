/*
 *  BermudaOS - StdIO - I/O read
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

/**
 * \brief Read from a file.
 * \param fd File descriptor.
 * \param buff Buffer to read.
 * \param size Size of the buffer.
 */
PUBLIC int read(int fd, void *buff, size_t size)
{
	return __iob[fd]->read(__iob[fd], buff, size);
}
