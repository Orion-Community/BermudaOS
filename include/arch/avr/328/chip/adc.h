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

#include <stdlib.h>
#include <arch/avr/io.h>

#define BermudaGetADMUX()   MEM_IO8(0x7C)
#define BermudaGetADCSRA()  MEM_IO8(0x7A)
#define BermudaGetADCL()    MEM_IO8(0x78)
#define BermudaGetADCH()    MEM_IO8(0x79)
#define BermudaGetADCSRB()  MEM_IO8(0x7B)
#define BermudaGetDIDR0()   MEM_IO8(0x7E)

struct adc;

/*
 * functions / variables
 */
extern void BermudaInitBaseADC();
extern void BermudaInitADC(struct adc*);

typedef unsigned short (*adc_read_t)(struct adc*);
typedef void (*adc_write_t)(struct adc*, unsigned short);

struct adc
{
        uint8_t id;

        adc_read_t read;
        adc_write_t write;
        void (*update)(struct adc*);
        
        volatile uint8_t *adcl, *adch,
                *admux, *adcsra, *adcsrb,
                *didr0;
} __attribute__((packed));

#endif
