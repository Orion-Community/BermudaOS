/*
 *  BermudaOS - AVR ATmega TWI module
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

//! \file include/arch/avr/twif.h

#ifndef __AVR_TWIF_H_
#define __AVR_TWIF_H_


#ifdef __cplusplus
extern "C"
{
#endif

extern int BermudaTwHwIfacBusy(TWIBUS *bus);
extern int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);

extern void BermudaTwiISR(TWIBUS *bus);
extern int BermudaTwiSlaveListen(TWIBUS *bus, size_t *num, void *rx, size_t rxlen, 
	unsigned int tmo);
extern int BermudaTwiSlaveRespond(TWIBUS *bus, const void *tx, size_t txlen,
	unsigned int tmo);
extern int BermudaTwiMasterTransfer(TWIBUS *twi, const void *tx, size_t txlen,  
	void *rx, size_t rxlen, unsigned char sla,
	uint32_t frq, unsigned int tmo);

extern void BermudaAvrTwiIrqAttatch(TWIBUS *bus, void (*handle)(TWIBUS*));
extern void BermudaAvrTwiIrqDetatch(TWIBUS *bus);

#ifdef __cplusplus
}
#endif


#endif