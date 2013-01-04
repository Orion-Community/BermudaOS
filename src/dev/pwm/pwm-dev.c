/*
 *  BermudaOS - PWM driver
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

#include <dev/dev.h>
#include <dev/pwm.h>

/**
 * \brief Open a PWM socket.
 * \param pwm PWM data structure to bind to.
 * \param flags Open flags.
 * \see mode
 */
PUBLIC int pwmdev_socket(struct pwm *pwm, uint16_t flags)
{
	return -1;
}

/**
 * \brief Write to a PWM socket.
 * \param stream I/O stream.
 * \param data Data to write.
 * \param len Length of data.
 * \see write
 */
PUBLIC int pwmdev_write(FILE *stream, const void *data, size_t len)
{
	return -1;
}

/**
 * \brief Read from a PWM socket.
 * \param stream I/O stream.
 * \param data Data stream.
 * \param len Length of data.
 * \see read
 */
PUBLIC int pwmdev_read(FILE *stream, void *data, size_t len)
{
	return -1;
}

/**
 * \brief Flush a PWM socket.
 * \param stream I/O stream.
 * \see flush
 */
PUBLIC int pwmdev_flush(FILE *stream)
{
	return -1;
}

/**
 * \brief Close a PWM socket.
 * \param stream I/O stream to close.
 * \see close
 */
PUBLIC int pwmdev_close(FILE *stream)
{
	return -1;
}
