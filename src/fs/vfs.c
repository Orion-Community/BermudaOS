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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <arch/io.h>

/**
 * \brief Array of files which are currently opened with fopen.
 */
FILE *__iob[MAX_OPEN];

/**
 * \brief List head of the virtual files.
 */
FILE *vfs_head = NULL;

/**
 * \brief Initialize the virtual file system.
 */
PUBLIC void vfs_init()
{
	int i = 3;
	
	for(; i < MAX_OPEN; i++) {
		__iob[i] = NULL; // initialise all entries to zero.
	}
	vfs_head = NULL;
}

/**
 * \brief Add a new file to the linked list.
 * \param f File to add.
 */
PUBLIC void vfs_add(FILE *f)
{
	f->next = vfs_head;
	vfs_head = f;
}

/**
 * \brief Delete a file from the inode head.
 * \param f File to delete.
 */
PUBLIC int vfs_delete(FILE *f)
{
	FILE **fpp;
	
	BermudaEnterCritical();
	fpp = &vfs_head;
	BermudaExitCritical();
	
	for(; *fpp; fpp = &(*fpp)->next) {
		if(*fpp == f) {
			*fpp = f->next;
			return 0;
		}
	}
	return -1;
}
