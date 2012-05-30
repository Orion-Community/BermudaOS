/*
 *  BermudaOS - Sensor subsystem
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

#ifndef __SENSORS_H
#define __SENSORS_H

/* function declarations */
extern char SensorRegister(struct sensor*);
extern char SensorUnregister(struct sensor*);


/* function pointer decls */
typedef void (*sensor_read_t)(struct sensor *);
typedef unsigned long (*sensor_write_t)(struct sensor*, unsigned long);

typedef enum
{
        LM35DH,
        LED
} sensor_t;

struct sensor
{
        void *pointer;

        unsigned long id;
        sensor_t type;

        sensor_read_t read;
        sensor_write_t write;
} __attribute__((packed));

#endif
