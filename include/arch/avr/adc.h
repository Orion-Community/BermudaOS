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

//! \file include/arch/avr/adc.h AVR ATmega Analog Digital Converter.


#ifndef __AVR_ADC_h
#define __AVR_ADC_h

#include <bermuda.h>

#include <dev/adc.h>
#include <arch/adc.h>
#include <arch/avr/io.h>

#define ADC_DEFAULT_AREF 1

extern struct adc adc0;

/*
 * functions / variables
 */
__DECL
#ifdef __EVENTS__
/**
 * \brief Attatch (enable) the hardware IRQ of the given ADC.
 * \param adc ADC to enable the interrupt for.
 * \note The 'I'-bit in the SREG register must be set to allow the hardware to
 *       raise an interrupt.
 */
static inline void BermudaAdcIrqAttatch(ADC *adc)
{
	*(adc->adcsra) |= BIT(ADIE);
}

/**
 * \brief Detatch (disable) the hardware IRQ of the given ADC.
 * \param adc ADC to disable the interrupt for.
 */
static inline void BermudaAdcIrqDetatch(ADC *adc)
{
	*(adc->adcsra) &= ~BIT(ADIE);
}
#endif

/**
 * \fn BermudaAdcEnable(struct adc *adc)
 * \brief Enable the ADC.
 * \param adc ADC to enable.
 *
 * This function enables the given ADC.
 */
static inline void BermudaAdcEnable(struct adc *adc)
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
static inline void BermudaAdcDisable(struct adc *adc)
{
        cpb(*adc->adcsra, ADEN);
}

extern void BermudaAdc0Init();
extern void BermudaAdcFactoryCreate(ADC *adc);
#ifdef __EVENTS__
extern unsigned short BermudaADCConvert(ADC *adc, unsigned char pin, unsigned int tmo);
#else
extern unsigned short BermudaADCConvert(ADC *adc, unsigned char pin);
#endif
__DECL_END

#endif /* __AVR_ADC_h */
