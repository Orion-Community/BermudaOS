/*
 * Arduino test project
 * (C) Michel Megens
 */

#ifndef __MAIN_H
#define __MAIN_H

#define FALSE 0
#define TRUE !FALSE

typedef enum
{
        E_SUCCESS,
        E_PANIC,
        E_ENDOFPROG
} error_t;

static int setup();
static int loop();

#define ArduinoAnalogRead(x) analogRead(x)
#define ArduinoWait(x) delay(x)
#define ArduinoDigitalRead(x) digitalRead(x)
#define ArduinoDigitalWrite(x, y) digitalWrite(x, y)

#endif
