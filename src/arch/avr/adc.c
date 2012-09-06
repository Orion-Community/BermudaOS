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

#include <bermuda.h>

#include <arch/io.h>
#include <arch/avr/interrupts.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#if defined(__ADC__) || defined(__DOXYGEN__)

struct adc adc0;

#ifdef __EVENTS__
volatile void *adc0_mutex = SIGNALED;
volatile void *adc0_queue = SIGNALED;
#endif

/**
 * \fn BermudaInitBaseADC()
 * \brief Initialise the main ADC.
 * 
 * This function initializes the main ADC and fills up a struct adc (BermADC).
 */
PUBLIC void BermudaInitADC0()
{
        struct adc* adc = &adc0;
        BermudaInitADC(adc);
        BermudaAdcEnable(adc);

        BermudaAdcSetPrescaler(adc, ADC_DEFAULT_CLK);
#ifdef THREADS
        BermudaAdcIrqAttatch(adc);
#endif
        return;
}

#endif