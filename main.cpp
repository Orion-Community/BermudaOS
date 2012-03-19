/*
 * Arduino test project
 * (C) Michel Megens
 */

#include <WProgram.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>


#include "main.h"

unsigned char DIGITAL_PINS[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_PIN  PINB
#define LED      PINB5

extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;

void flash_led(uint8_t count)
{
	while (count--) {
		LED_PORT |= _BV(LED);
		_delay_ms(100);
		LED_PORT &= ~_BV(LED);
		_delay_ms(500);
	}
}

int main(void)
{
	LED_DDR = 0xff;
        flash_led(3);
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
                retval = loop();
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

/* Calculates the size of the free list */
int BermFreeListSize() {
        struct __freelist* current;
        int total = 0;

        for (current = __flp; current; current = current->nx) 
        {
                total += 2; /* Add two bytes for the memory block's header  */
                total += (int) current->sz;
        }

        return total;
}

int BermFreeMemory() 
{
        int free_memory;

        if ((int)__brkval == 0) 
        {
                free_memory = ((int)&free_memory) - ((int)&__heap_start);
        } 
        else 
        {
                free_memory = ((int)&free_memory) - ((int)__brkval);
                free_memory += BermFreeListSize();
        }
        return free_memory;
}

static int
setup()
{
        pinMode(DIGITAL_PINS[13], OUTPUT);
        ArduinoWait(50);
        ArduinoDigitalWrite(DIGITAL_PINS[13], LOW);
        pinMode(A0, INPUT);
        Serial.begin(9600);
        Serial.println("test");
        LED_DDR |= _BV(LED);
        return E_SUCCESS;
}

static int
loop()
{
        float raw_temp = ArduinoAnalogRead(A0);
        float temp = raw_temp / 1024 * 5000;
 
        Serial.print("Temperature: ");
        Serial.println(temp/10);
        ArduinoWait(1000);
        return E_SUCCESS;
}


