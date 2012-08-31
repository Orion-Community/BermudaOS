/*
 *  BermudaOS - Device administration
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

#include <arch/types.h>

#include <dev/twif.h>
#include <dev/dev.h>

#include <fs/vfile.h>

int BermudaTwiDevWrite(VFILE *file, const void *tx, size_t size);
int BermudaTwiDevRead(VFILE *file, void *rx, size_t size);

PUBLIC DEVICE *BermudaTwiDevInit(TWIBUS *bus, char *name)
{
	DEVICE *dev = BermudaHeapAlloc(sizeof(DEVICE));
	VFILE *file = BermudaHeapAlloc(sizeof(VFILE));
	dev->name = name;
	BermudaDeviceRegister(dev, bus);
	dev->io = file;
	
	file->write = &BermudaTwiDevWrite;
	file->read = &BermudaTwiDevRead;
	file->flush = NULL;
	file->close = NULL;
	file->data = dev;
	
	return dev;
}

PUBLIC int BermudaTwiDevWrite(VFILE *file, const void *tx, size_t size)
{
	return -1;
}

PUBLIC int BermudaTwiDevRead(VFILE *file, void *rx, size_t size)
{
	return -1;
}