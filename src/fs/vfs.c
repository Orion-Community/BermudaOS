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

#include <bermuda.h>
#include <lib/string.h>

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <fs/vfile.h>
#include <fs/vfs.h>

/**
 * \def MAX_OPEN
 * \brief Maximum amount of files opened at the same time.
 */
#define MAX_OPEN 16

/**
 * \brief Array of files which are currently opened with fopen.
 */
FILE *__iob[MAX_OPEN];

/**
 * \brief List head of the virtual files.
 */
static FILE *vfs_head = NULL;

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
PUBLIC void vfs_add(VFILE *f)
{
	f->next = vfs_head;
	vfs_head = f;
}

/**
 * \brief Delete a file from the inode head.
 * \param f File to delete.
 */
PUBLIC void vfs_delete(VFILE *f)
{
	//TODO: implement.
}

/**
 * \brief Open a file.
 * \param fname Name of the file to open.
 * \param mode File mode to use.
 * \return The file descriptor.
 */
PUBLIC int fdopen(char *fname, unsigned char mode)
{
	int i = 0;
	for(; i < MAX_OPEN; i++) {
		if(!strcmp(__iob[i]->name, fname)) {
			fdmode(i, mode);
			return i;
		}
	}
	return -1;
}

/**
 * \brief Change the file mode of a given file.
 * \param fd File descriptor of the file to change.
 * \param mode New file mode.
 */
PUBLIC void fdmode(int fd, unsigned char mode)
{
	__iob[fd]->mode = mode;
}

/**
 * \brief Write to a file.
 * \param fd File descriptor.
 * \param buff Buffer to write.
 * \param size Size of the buffer.
 */
PUBLIC int write(int fd, const void *buff, size_t size)
{
	VFILE *file = __iob[fd];
	
	return file->write(file, buff, size);
}

/**
 * \brief Read from a file.
 * \param fd File descriptor.
 * \param buff Buffer to read.
 * \param size Size of the buffer.
 */
PUBLIC int read(int fd, void *buff, size_t size)
{
	return __iob[fd]->read(__iob[fd], buff, size);
}
