/*
 *  BermudaOS - Device errors
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
 * \file include/dev/error.h Device error definitions.
 * \brief Defines error codes for devices.
 */

#ifndef __DEV_ERROR_H
#define __DEV_ERROR_H

/**
 * \enum dev_error
 * \brief Defines several errors a device driver can output.
 * 
 * \typedef dev_error_t
 * \brief Type definition of the dev_error.
 * \see dev_error
 */
typedef enum dev_error
{
	DEV_OK, //!< No errors have occurred.
	DEV_ERROR, //!< General error has occurred.
	DEV_NULL, //!< Null pointer detected.
	DEV_NOINIT, //!< Not initialized pointer detected.
	DEV_ALREADY_INITIALIZED, //!< The device is already initialized.
	DEV_OUTOFBOUNDS, //!< Defines that an array is accessed out of its bounds.
	DEV_BROKEN, //!< Hardware is probably broken and there is no way that this devices is ever going to work.
	DEV_INTERNAL, //!< Internal device error. This is the moment to sent an e-mail to the developer.
} dev_error_t;

/**
 * \brief Cast a device error to a pointer.
 * \param __x Value to cast.
 * \note \p __x is casted to a void pointer.
 */
#define PTR_ERROR(__x) ((void*)__x)

/**
 * \brief Cast a type to the the dev_error_t type.
 * \param __x Value to cast to dev_error_t.
 */
#define DEV_ERROR(__x) ((dev_error_t)__x)

#endif /* __DEV_ERROR_H */