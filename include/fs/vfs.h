/*
 *  BermudaOS - VFS driver.
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

#ifndef __VFS_H
#define __VFS_H

#include <bermuda.h>

#include <fs/vfile.h>

__DECL
extern void vfs_init();
extern int fdopen(char *fname, unsigned char mode);
extern void fdmode(int fd, unsigned char mode);

extern int write(int fd, const void *buff, size_t size);
extern int read(int fd, void *buff, size_t size);

extern void vfs_add(VFILE *f);
extern int vfs_delete(VFILE *f);
__DECL_END

#endif /* __VFS_H */