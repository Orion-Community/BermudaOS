/*
 *  BermudaOS - TWI device
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

//! \file include/dev/twidev.h TWI device header.

#ifndef __TWI_DEV__
#define __TWI_DEV__

#include <bermuda.h>
#include <dev/twif.h>

extern TWIMSG *BermudaTwiMsgCompose(const void *tx, size_t txlen, void *rx, size_t rxlen, 
									unsigned char sla, uint32_t scl, unsigned int tmo, 
									twi_call_back_t call_back);
extern void BermudaTwiMsgDestroy(TWIMSG *msg);
extern DEVICE *BermudaTwiDevInit(TWIBUS *bus, char *name);
extern int BermudaTwiDevWrite(VFILE *file, const void *tx, size_t size);
extern int BermudaTwiDevRead(VFILE *file, void *rx, size_t size);

/**
 * \brief Shortcut to define a correct TWI call back function.
 * \param fn Function name.
 * \param arg Function argument name (which is the used TWI message).
 * \see TWIMSG
 */
#define TWI_HANDLE(fn, arg) \
static void fn(TWIMSG *arg); \
static void fn(TWIMSG *arg)

#endif