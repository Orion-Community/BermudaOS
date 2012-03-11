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

#ifndef __ADC_H
#define __ADC_H

#include <avr/io.h>

struct adc;

/*
 * functions / variables
 */
extern void BermInitBaseADC();
extern void BermInitADC(struct adc*);

/*
 * private functions
 */
// static void BermADCWrite(struct adc*, unsigned short);
// static unsigned short BermADCRead(struct adc*);
static void BermADCUpdate(struct adc*);

typedef unsigned short (*adc_read_t)(struct adc*);
typedef void (*adc_write_t)(struct adc*, unsigned short);

struct adc
{
        uint8_t id;

        adc_read_t read;
        adc_write_t write;
        void (*update)(struct adc*);
        
        uint8_t adcl, adch,
                admux, adcsra, adcsrb,
                didr0;
} __attribute__((packed));

#endif
