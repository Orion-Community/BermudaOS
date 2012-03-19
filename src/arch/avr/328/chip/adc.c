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
#include <lib/binary.h>

/*
 * private functions
 */
// static void BermADCWrite(struct adc*, unsigned short);
static unsigned short BermudaADCConvert(unsigned char);

struct adc BermADC;
static unsigned char adc_ref = 1; /* default analog reference */

/**
 * \fn BermudaInitBaseADC()
 * \brief Initialise the main ADC.
 * 
 * This function initializes the main ADC and fills up a struct adc (BermADC).
 */
void BermudaInitBaseADC()
{
        struct adc* adc = &BermADC;
        adc->id = 0;
        BermudaInitADC(adc);
        spb(*adc->adcsra, ADEN);
        spb(*adc->adcsra, ADIE);
        spb(*adc->adcsra, ADPS1);
        spb(*adc->adcsra, ADPS2);
        return;
}

/**
 * \fn BermudaInitADC(struct adc*)
 * \brief Fill up a struct adc.
 * \param adc The ADC structure to initialize.
 * 
 * Initializes an adc struct.
 */
void BermudaInitADC(adc)
struct adc* adc;
{
        adc->adcl = &BermudaGetADCL();
        adc->adch = &BermudaGetADCH();
        adc->admux = &BermudaGetADMUX();
        adc->adcsra = &BermudaGetADCSRA();
        adc->adcsrb = &BermudaGetADCSRB();
        adc->didr0 = &BermudaGetDIDR0();
        adc->read = &BermudaADCConvert;
        return;
}

static unsigned short BermudaADCConvert(pin)
unsigned char pin;
{
        if((BermudaGetADCSRA() & BIT(ADEN)) == 0)
                return 0;
        
        struct adc *adc = &BermADC;
        while((*adc->adcsra & BIT(ADSC)) != 0);
        
        /* select input channel */
        *adc->admux = ((pin & 0x7) | (adc_ref << 6));
        
        /* start conversion */
        spb(*adc->adcsra, ADSC);

        /* wait for it to finish */
#ifdef THREADS
        BermudaThreadSleep();
#else
        while((*adc->adcsra & BIT(ADSC)) != 0);
#endif
        /* finished, get results and return them */
        unsigned char low = *adc->adcl;
        unsigned char high = *adc->adch;
        return low | (high << 8);
}

/**
 * \fn
 * \brief ADC ISR
 * 
 * This ISR is called by hardware when an ADC conversion is complete.
 */
SIGNAL(ADC_vect)
{
        printf("test\n");
        return;
}
