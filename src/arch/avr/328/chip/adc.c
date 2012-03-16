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

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <arch/avr/io.h>

/*
 * private functions
 */
// static void BermADCWrite(struct adc*, unsigned short);
// static unsigned short BermADCRead(struct adc*);

static struct adc BermADC;

void BermudaInitBaseADC()
{
        struct adc* adc = &BermADC;
        BermudaInitADC(adc);
        spb(*adc->adcsra, ADEN);
        spb(*adc->adcsra, ADIE);
        spb(*adc->adcsra, ADPS1);
        spb(*adc->adcsra, ADPS2);
        printf("Address of the main ADC %p\n", adc);
        return;
}

void BermudaInitADC(adc)
struct adc* adc;
{
        adc->adcl = &BermudaGetADCL();
        adc->adch = &BermudaGetADCH();
        adc->admux = &BermudaGetADMUX();
        adc->adcsra = &BermudaGetADCSRA();
        adc->adcsrb = &BermudaGetADCSRB();
        adc->didr0 = &BermudaGetDIDR0();
        return;
}

SIGNAL(ADC_vect)
{
        return;
}
