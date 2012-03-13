/*
 * Arduino test project
 * (C) Michel Megens
 */

#include <WProgram.h>
#include "main.h"

unsigned char DIGITAL_PINS[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };


extern unsigned int __heap_start;
extern void *__brkval;

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;

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
        ArduinoDigitalWrite(DIGITAL_PINS[13], HIGH);
        pinMode(A0, INPUT);
        Serial.begin(9600);
        Serial.println("test");
        return E_SUCCESS;
}

static unsigned char flip = 0;
static int
loop()
{
        Serial.println("[0] Main thread");
        return E_SUCCESS;
}


