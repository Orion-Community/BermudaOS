/*
 *  BermudaOS - Digital I/O
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

#ifndef __DIGITAL_IO_H
#define __DIGITAL_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#define ROM __attribute__((__progmem__))

extern const unsigned char ROM *port_to_output;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DIGITAL_IO_H */
