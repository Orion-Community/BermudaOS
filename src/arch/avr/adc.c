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

//! \file src/arch/avr/adc.c Analog Digital Converter.

#include <bermuda.h>

#include <dev/adc.h>

#include <arch/io.h>
#include <arch/adc.h>
#include <arch/avr/interrupts.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#if defined(__ADC__) || defined(__DOXYGEN__)

// private functions
PRIVATE WEAK void BermudaAdcSetAnalogRef(ADC *adc, const unsigned char aref);
PRIVATE WEAK int BermudaAdcSetPrescaler(ADC *adc, const unsigned char prescaler);
PRIVATE WEAK int BermudaAdcSetPrescaler(ADC *adc, const unsigned char prescaler);

struct adc adc0;

#ifdef __EVENTS__
static void *adc0_mutex = SIGNALED;
static void *adc0_queue = SIGNALED;
#endif

/**
 * \fn BermudaInitBaseADC()
 * \brief Initialise the main ADC.
 * 
 * This function initializes the main ADC and fills up a struct adc (BermADC).
 */
PUBLIC void BermudaAdc0Init()
{
	struct adc* adc = &adc0;
	BermudaAdcFactoryCreate(adc);
	BermudaAdcEnable(adc);

	BermudaAdcSetPrescaler(adc, ADC_DEFAULT_CLK);
#ifndef __EVENTS__
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
PUBLIC void BermudaAdcFactoryCreate(ADC *adc)
{
	adc->adcl = &BermudaGetADCL();
	adc->adch = &BermudaGetADCH();
	adc->admux = &BermudaGetADMUX();
	adc->adcsra = &BermudaGetADCSRA();
	adc->adcsrb = &BermudaGetADCSRB();
	adc->didr0 = &BermudaGetDIDR0();
	adc->read = &BermudaADCConvert;
	adc->aref = ADC_DEFAULT_AREF;
	
#ifdef __EVENTS__
	adc->mutex = &adc0_mutex;
	adc->queue = &adc0_queue;
#endif
	return;
}

/**
 * \fn BermudaADCConvert(pin)
 * \brief Read the ADC.
 * \param pin The ADC pin to read from.
 * \return The converted value.
 * \todo Make it board independent.
 *
 * This function starts the analog to digital conversion and returns the
 * result.
 */
#ifdef __EVENTS__
PUBLIC unsigned short BermudaADCConvert(ADC *adc, unsigned char pin, unsigned int tmo)
#else
PUBLIC unsigned short BermudaADCConvert(ADC *adc, unsigned char pin)
#endif
{
#ifdef __BOARD__
	pin = BermudaBoardAnalogPinAdjust(pin);
#endif
#ifdef __EVENTS__
	BermudaEventWait((volatile THREAD**)adc->mutex, tmo);
#endif
	if(((*(adc->adcsra)) & BIT(ADEN)) == 0)
		return 0;
	
	/* select input channel */
	*adc->admux = ((pin & 0x7) | (adc->aref << 6));
	
	/* start conversion */
	spb(*adc->adcsra, ADSC);

	/* wait for it to finish */
#ifdef __EVENTS__
	BermudaEventWaitNext( (volatile THREAD**)adc->queue, tmo);
#else
	while((*adc->adcsra & BIT(ADSC)) != 0);
#endif

	/* finished, get results and return them */
	unsigned char low = *adc->adcl;
	unsigned char high = *adc->adch;

#ifdef __EVENTS__
	BermudaEventSignal((volatile THREAD**)adc->mutex);
#endif
	return low | (high << 8);
}

/**
 * \fn BermudaAdcSetPrescaler(ADC *adc)
 * \brief Set the CLK prescaler.
 * \param adc ADC to configure.
 * \param prescaler Prescaler to set.
 *
 * prescaler[0] := ADSP0
 * prescaler[1] := ADSP1
 * prescaler[2] := ADSP2
 */
PRIVATE WEAK int BermudaAdcSetPrescaler(ADC *adc, const unsigned char prescaler)
{
	*(adc->adcsra) &= ~B111; // clear all prescaler bits
	*(adc->adcsra) |= (prescaler & B111);
	return 0;
}

#ifdef __EVENTS__
SIGNAL(ADC_CC_vect)
{
	BermudaEventSignalFromISR((volatile THREAD**)ADC0->queue);
}
#endif

#endif
