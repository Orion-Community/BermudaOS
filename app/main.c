/*
 *  BermudaOS - App main
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
#include <string.h>

#include <sys/thread.h>
#include <sys/virt_timer.h>
#include <sys/epl.h>
#include <sys/events/event.h>

#include <lib/spiram.h>
#include <lib/24c02.h>

#include <dev/dev.h>
#include <dev/spibus.h>
#include <dev/adc.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/usart/usart.h>
#include <dev/i2c/busses/atmega.h>
#include <dev/usart/busses/atmega_usart.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <arch/io.h>
#include <arch/adc.h>
#include <arch/avr/pgm.h>
#include <arch/avr/328/dev/spibus.h>

#include <net/core/dev.h>

static VTIMER *timer;
struct i2c_client eeprom_client;
static char epl_stack[128];
THREAD epl_thread;

volatile void *epl_mutex = SIGNALED;
DEF_EPL(i2c_epl, epl_mutex);

#ifdef __THREADS__

THREAD(epl_test, arg)
{
	struct epl_list_node *node;
	int i = 0;
	while(1) {
		if(epl_lock(&i2c_epl) == 0) {
			node = malloc(sizeof(*node));
			node->data = (void*)i;
			i++;
			epl_add_node(&i2c_epl, node, EPL_APPEND);
			epl_unlock(&i2c_epl);
		}
		BermudaThreadSleep(2000);
	}
}

#endif

static unsigned char led = 1;

PUBLIC void TestTimer(VTIMER *timer, void *arg)
{
	BermudaDigitalPinWrite(5, led);
	led ^= 1;
}

void setup()
{
	printf_P(PSTR("Booting!\n"));
	BermudaSetPinMode(A0, INPUT);
	BermudaSetPinMode(5, OUTPUT);
#ifdef __THREADS__
	int fd;
	char buff[4];

	while(1) {
		fd = usartdev_socket(USART0, "USART0", _FDEV_SETUP_RW);
		if(fd < 0) {
			_exit();
		}
		read(fd, buff, 3);
		usartdev_close(fd);
		buff[3] = '\0';
		if(!strcmp(buff, "run")) {
			break;
		}
		BermudaThreadSleep(500);
	}
	
	BermudaThreadCreate(&epl_thread, "EPL", &epl_test, NULL, 128,
						&epl_stack[0], BERMUDA_DEFAULT_PRIO);
#endif
	timer = BermudaTimerCreate(500, &TestTimer, NULL, BERMUDA_PERIODIC);
	atmega_i2c_init_client(&eeprom_client, ATMEGA_I2C_C0);
	Bermuda24c02Init(&eeprom_client);
	BermudaSpiRamInit(SPI0, 10);
	BermudaSpiRamWriteByte(0x50, 0xF8);
	Bermuda24c02WriteByte(100, 0xAC);
}

#ifdef __THREADS__
void loop()
#else
unsigned long loop()
#endif
{
	float tmp = 0;
	int temperature = 0;
	unsigned char read_back_eeprom = 0, read_back_sram = 0;
	struct epl_list_node *car;
	
	tmp = ADC0->read(ADC0, A0, 500);
	temperature = tmp / 1024 * 5000;
	temperature /= 10;
	
	read_back_sram = BermudaSpiRamReadByte(0x50);
	read_back_eeprom = Bermuda24c02ReadByte(100);

	printf_P(PSTR("T=%u M=%X E=%X S=%X\n"), temperature, BermudaHeapAvailable(), read_back_eeprom,
				read_back_sram);
	
	if(epl_lock(&i2c_epl) == 0) {
		for(car = (&i2c_epl)->nodes; car != NULL; car = car->next) {
			printf("%p ", car->data);
			if(car->next == NULL) {
				putc('\n', stdout);
				if(epl_entries(&i2c_epl)) {
// 					epl_delete_node_at(&i2c_epl, epl_entries(&i2c_epl)-1);
					printf("Entries: %u\n", epl_entries(&i2c_epl));
				}
				break;
			}
		}
		epl_unlock(&i2c_epl);
	}

#ifdef __THREADS__
	BermudaThreadSleep(5000);
	return;
#else
	return 500;
#endif
}
