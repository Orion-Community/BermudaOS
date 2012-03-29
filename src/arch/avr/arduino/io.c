/*
 *  BermudaOS - Arduino specific I/O module
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
#include <avr/interrupt.h>
#include <arch/avr/io.h>
#include <arch/avr/arduino/io.h>
#include <lib/binary.h>

const unsigned char ROM BermudaPinToPort[] =
{
        PD, PD, PD, PD, PD, PD, PD, PD, /* pin 0-7  */
        PB, PB, PB, PB, PB, PB,         /* pin 8-13 */
        PC, PC, PC, PC, PC,             /* ADC 0-5  */
};

const unsigned char ROM BermudaPinToMask[] =
{
        BIT(0), BIT(1), BIT(2), BIT(3), BIT(4), BIT(5),
        BIT(6), BIT(7), /* PD0 - PD7 */
        BIT(0), BIT(1),BIT(2), BIT(3),BIT(4), BIT(5), /* PB0 - PB5 */
        BIT(0), BIT(1),BIT(2), BIT(3),BIT(4), BIT(5), /* PC0 - PC5 */
};

void BermudaSetPinMode(pin, mode)
unsigned char pin, mode;
{
        unsigned char mask = BermudaGetIOMask((unsigned short)pin);
        unsigned char port = BermudaGetIOPort((unsigned short)pin);
        volatile unsigned char *modeReg = BermudaGetIOMode(port);
        
        if(mode == INPUT)
        {
                cli();
                *modeReg &= ~mask;
                sei();
                return;
        }
        else
        {
                cli();
                *modeReg |= mask;
                sei();
                return;
        }
}

void BermudaDigitalPinWrite(unsigned char pin, unsigned char value)
{
        unsigned char mask = BermudaGetIOMask(pin);
        unsigned char port = BermudaGetIOPort(pin);
        volatile unsigned char *out = BermudaGetOuputReg(port);

        if(PIN_NOT_AVAILABLE == port)
                return;

        if(value == INPUT)
        {
                *out &= ~mask;
        }
        else
        {
                *out |= mask;
        }
}

unsigned char BermudaDigitalPinRead(unsigned char pin)
{
        unsigned char bit = BermudaGetIOMask(pin);
        unsigned char port = BermudaGetIOPort(pin);
        volatile unsigned char *in = BermudaGetInputReg(port);

        if(PIN_NOT_AVAILABLE == port)
                return LOW;

        if((*in & bit) != 0)
                return HIGH;
        else
                return LOW;
}
