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

#ifndef __TWI_DEV__
#define __TWI_DEV__

#include <bermuda.h>
#include <dev/twif.h>

extern TWIMSG *BermudaTwiMsgCompose(const void *tx, uptr txlen, void *rx, uptr rxlen, 
									unsigned char sla, uint32_t scl, unsigned int tmo, 
									twi_call_back_t call_back);
extern void BermudaTwiMsgDestroy(TWIMSG *msg);
extern DEVICE *BermudaTwiDevInit(TWIBUS *bus, char *name);

#endif