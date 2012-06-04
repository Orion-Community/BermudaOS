/*
 *  BermudaOS - TWI interface
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

//! \file arch/avr/328/dev/twibus.c Hardware TWI bus controller.

#include <bermuda.h>

#include <dev/twif.h>
#include <arch/avr/328/dev/twibus.h>

/**
 * \brief Set the status register.
 * \param twi Bus to set the new status for.
 * \warning Should only be called from the TWI ISR.
 * \return The updated status value.
 * 
 * Updates the status regsiter in the TWI bus using the hardware status register.
 */
PUBLIC inline uint8_t BermudaTwiUpdateStatus(TWIBUS *twi)
{
	TWIHW *hwio = twi->hwio;
	twi->status = (*(hwio->twsr)) & B11;
	return twi->status;
}