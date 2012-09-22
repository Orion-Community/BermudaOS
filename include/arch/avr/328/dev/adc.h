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

//! \file include/arch/avr/328/dev/adc.h Analog Digital Converter.

#ifndef __ATmega328_ADC_H
#define __ATmega328_ADC_H

#include <bermuda.h>
#include <arch/avr/io.h>

#define BermudaGetADMUX()   MEM_IO8(0x7C)
#define BermudaGetADCSRA()  MEM_IO8(0x7A)
#define BermudaGetADCL()    MEM_IO8(0x78)
#define BermudaGetADCH()    MEM_IO8(0x79)
#define BermudaGetADCSRB()  MEM_IO8(0x7B)
#define BermudaGetDIDR0()   MEM_IO8(0x7E)

#define ADEN 7
#define ADSC 6
#define ADIE 3

#define ADC_DEFAULT_CLK              B110

/**
 * \def ADC0
 * \brief Can be used to address the structure of ADC0.
 */
#define ADC0 (&adc0)

#endif /* __ATmega328_ADC_H */
