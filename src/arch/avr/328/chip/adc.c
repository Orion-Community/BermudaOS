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

/** \file adc.c */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <lib/binary.h>
#include <avr/interrupt.h>
#include <arch/avr/io.h>
#include <lib/binary.h>

struct adc BermADC;
static unsigned char adc_ref = 1; /* default analog reference */

#ifdef THREADS
static THREAD *BermudaADCThread;
#endif

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
        BermudaAdcEnable(adc);

        BermudaAdcSetPrescaler(adc, ADC_DEFAULT_CLK);
#ifdef THREADS
        BermudaAdcIrqAttatch(adc);
#endif
        
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

/**
 * \fn BermudaADCConvert(pin)
 * \brief Read the ADC.
 * \param pin The ADC pin to read from.
 * \return The converted value.
 *
 * This function starts the analog to digital conversion and returns the
 * result.
 */
PRIVATE unsigned short BermudaADCConvert(pin)
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
 * \fn BermudaAdcSetAnalogRef(adc, aref)
 * \brief Set the analog reference.
 * \param adc Analog Digital Converter.
 * \param aref The analog reference.
 *
 * 0 | AREF, Internal Vref turned off
 * 1 | AVCC with external capacitor at AREF pin
 * 2 | Invalid
 * 3 | Internal 1.1V Voltage Reference with external capacitor at AREF pin
 */
PRIVATE WEAK inline void BermudaAdcSetAnalogRef(adc, aref)
struct adc *adc;
unsigned char aref;
{
        if(2 == aref)
                return;

        *(adc->admux) = (aref << 6);
        adc->aref = aref;
}

/**
 * \fn BermudaAdcEnable(struct adc *adc)
 * \brief Enable the ADC.
 * \param adc ADC to enable.
 *
 * This function enables the given ADC.
 */
PRIVATE inline void BermudaAdcEnable(struct adc *adc)
{
        spb(*adc->adcsra, ADEN);
}

/**
 * \fn BermudaAdcEnable(struct adc *adc)
 * \brief Disable the ADC.
 * \param adc ADC to disable.
 *
 * This function disables the given ADC.
 */
PRIVATE inline void BermudaAdcDisable(struct adc *adc)
{
        cpb(*adc->adcsra, ADEN);
}

/**
 * \fn BermudaAdcSetPrescaler(ADC *adc)
 * \brief Set the CLK prescaler.
 * \param adc ADC to configure.
 * \param prescaler Prescaler to set.
 *
 * Set ADC clock prescaler. The prescaler must be a power of two. The minimal
 * prescaler is two, when a value of 0 is given the prescaler is set to two.
 */
PRIVATE inline int BermudaAdcSetPrescaler(struct adc *adc, unsigned char prescaler)
{
        if(!BermudaIsPowerOfTwo(prescaler))
                return -1;
        else
                adc->prescaler = (prescaler) ? prescaler : 2;

        switch(prescaler)
        {
                case 2:
                        cpb(*adc->adcsra, ADPS0);
                        cpb(*adc->adcsra, ADPS1);
                        cpb(*adc->adcsra, ADPS2);
                        break;

                case 4:
                        spb(*adc->adcsra, ADPS0);
                        cpb(*adc->adcsra, ADPS1);
                        cpb(*adc->adcsra, ADPS2);
                        break;

                case 8:
                        spb(*adc->adcsra, ADPS0);
                        spb(*adc->adcsra, ADPS1);
                        cpb(*adc->adcsra, ADPS2);
                        break;

                case 16:
                        cpb(*adc->adcsra, ADPS0);
                        cpb(*adc->adcsra, ADPS1);
                        spb(*adc->adcsra, ADPS2);
                        break;

                case 32:
                        spb(*adc->adcsra, ADPS0);
                        cpb(*adc->adcsra, ADPS1);
                        spb(*adc->adcsra, ADPS2);
                        break;

                case 64:
                        printf("hoi");
                        cpb(*adc->adcsra, ADPS0);
                        spb(*adc->adcsra, ADPS1);
                        spb(*adc->adcsra, ADPS2);
                        break;

                case 128:
                        spb(*adc->adcsra, ADPS0);
                        spb(*adc->adcsra, ADPS1);
                        spb(*adc->adcsra, ADPS2);
                        break;

                default:
                        BermudaAdcSetPrescaler(adc, ADC_DEFAULT_CLK);
                        break;
        }
        return 0;
}

#ifdef THREADS
/**
 * \fn BermudaAdcIrqAttatch(struct adc *adc)
 * \brief Attatch the ADC IRQ.
 * \param adc The ADC which interrupt should be enabled.
 *
 * The ADC completion hw IRQ will be active after calling this function. The pre-
 * defined vector for this ISR is <i>ADC_vect</i>.
 */
PRIVATE void BermudaAdcIrqAttatch(struct adc *adc)
{
        spb(*adc->adcsra, ADIE);
}

/**
 * \fn BermudaAdcIrqDetatch(struct adc *adc)
 * \brief Disable the ADC IRQ.
 * \param adc The ADC which IRQ should be disabled.
 *
 * The ADC completion hw IRQ will be deactivated after calling this function.
 */
PRIVATE void BermudaAdcIrqDetatch(struct adc *adc)
{
        cpb(*adc->adcsra, ADIE);
}

/**
 * \fn
 * \brief ADC ISR
 * 
 * This ISR is called by hardware when an ADC conversion is complete.
 */
SIGNAL(ADC_vect)
{
        return;
}
#endif
