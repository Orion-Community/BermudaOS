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

//! \file include/dev/adc.h Analog Digital Converter.

#ifndef __DEV_ADC_H
#define __DEV_ADC_H

#include <bermuda.h>

struct adc;

#ifdef __EVENTS__
/**
 * \brief Type definintion of the adc read function.
 * \param adc ADC structure.
 * \param pin Pin to read from.
 * \param tmo Maximum time-out.
 */
typedef unsigned short (*adc_read_t)(struct adc *adc, unsigned char pin, unsigned int tmo);
#else
/**
 * \brief Type definintion of the adc read function.
 * \param adc ADC structure.
 * \param pin Pin to read from.
 */
typedef unsigned short (*adc_read_t)(struct adc *adc,unsigned char pin);
#endif

/**
 * \brief ADC interface.
 */
struct adc
{
#ifdef __EVENTS__
	volatile void *mutex; //!< Queue to wait in.
	volatile void *queue; //!< Transfer waiting queue.
#endif
	adc_read_t read; //!< Function pointer which reads the ADC.

	unsigned char prescaler; //!< ADC internal clock prescaler.
	unsigned char aref; //!< Analog reference settings.
	
	volatile uint8_t *adcl, *adch,
			*admux, *adcsra, *adcsrb,
			*didr0;
} __attribute__((packed));

/**
 * \brief Type definition of the ADC structure.
 */
typedef struct adc ADC;

#endif