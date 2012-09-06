/*
 *  BermudaOS - Analog Digital Converter
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

#ifndef __DEV_ADC_H
#define __DEV_ADC_H

#include <bermuda.h>

struct adc;

#ifdef __EVENTS__
typedef unsigned short (*adc_read_t)(struct adc *,unsigned char, unsigned int);
#else
typedef unsigned short (*adc_read_t)(struct adc *,unsigned char);
#endif

/**
 * \brief ADC interface.
 */
struct adc
{
#ifdef __EVENTS__
	volatile void *mutex;
	volatile void *queue;
#endif
	adc_read_t read; //!< Function pointer which reads the ADC.

	unsigned char prescaler; //!< ADC internal clock prescaler.
	unsigned char aref; //!< Analog reference settings.
	
	volatile uint8_t *adcl, *adch,
			*admux, *adcsra, *adcsrb,
			*didr0;
} __attribute__((packed));

typedef struct adc ADC;

extern struct adc BermADC;

#endif