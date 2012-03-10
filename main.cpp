/*
 * Arduino test project
 * (C) Michel Megens
 */

#include <WProgram.h>
#include "main.h"

unsigned char DIGITAL_PINS[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };

int main(void)
{
        /*
         * initialise the arduino framework
         */
        init();

        /*
         * initialise our own framework
         */
        setup();

        int retval = 0;
        while(TRUE)
        {
                loop();
        }

        if(retval == E_ENDOFPROG)
                return 0;
        else
                return 1;
}

extern "C" void serial_write_string(char *data)
{
        Serial.println(data);
}

static int
setup()
{
        pinMode(DIGITAL_PINS[13], OUTPUT);
        pinMode(A0, INPUT);
        Serial.begin(9600);
        return E_SUCCESS;
}

static unsigned char flip = 0;
static int
loop()
{
        ArduinoDigitalWrite(DIGITAL_PINS[13], HIGH);
        ArduinoWait(500);
        
        ArduinoDigitalWrite(DIGITAL_PINS[13], LOW);
        float raw_temp = ArduinoAnalogRead(A0);
        float temp = raw_temp / 1024 * 5000;
        
        Serial.println(temp/10);
        ArduinoWait(500);
        return E_SUCCESS;
}
