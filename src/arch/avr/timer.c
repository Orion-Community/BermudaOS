/*
 *  BermudaOS - AVR timer functions
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

#include <lib/binary.h>

#include <arch/io.h>
#include <arch/avr/timer.h>

PUBLIC void BermudaAvrTimerSetISR(TIMER *timer, unsigned char isr)
{
	unsigned char i = 1;
	unsigned char int_mask;
	
	for(; i <= TIMER_ISRS; i << 1) {
		inb(timer->int_mask, &int_mask);
		if((isr & i) != 0) {
			outb(timer->int_mask, int_mask | i);
		}
	}
}



