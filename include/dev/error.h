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

#ifndef __DEV_ERROR_H
#define __DEV_ERROR_H

/**
 * \enum dev_error
 * \brief Defines several errors a device driver can output.
 */
typedef enum dev_error
{
	DEV_OK, //!< No errors have occurred.
	DEV_ERROR, //!< General error has occurred.
	DEV_NULL, //!< Null pointer detected.
	DEV_NOINIT, //!< Not initialized pointer detected.
	DEV_ALREADY_INITIALIZED, //!< The device is already initialized.
	DEV_OUTOFBOUNDS,
	DEV_BROKEN, //!< Hardware is probably broken and there is no way that this devices is ever going to work.
	DEV_INTERNAL, //!< Internal device error. This is the moment to sent an e-mail to the developer.
} dev_error_t;

#endif /* __DEV_ERROR_H */