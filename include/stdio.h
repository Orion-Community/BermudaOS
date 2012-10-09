/*
 *  BermudaOS - StdIO
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

#ifndef __STDIO_H
#define __STDIO_H

#include <bermuda.h>
#include <stddef.h>
#include <fs/vfs.h>

/**
 * \def MAX_OPEN
 * \brief Maximum amount of files opened at the same time.
 */
#define MAX_OPEN 16

/* file flags */
#define __SRD	0x0001		/* OK to read */
#define __SWR	0x0002		/* OK to write */
#define __SRWB  0x0004		/* Read/write from buffer */
#define __SPGM	0x0008		/* fmt string is in progmem */
#define __SERR	0x0010		/* found error */
#define __SEOF	0x0020		/* found EOF */
#define __SUNGET 0x040		/* ungetc() happened */
#define __SMALLOC 0x80		/* handle is malloc()ed */

#define _FDEV_SETUP_READ  __SRD	/**< fdev_setup_stream() with read intent */
#define _FDEV_SETUP_WRITE __SWR	/**< fdev_setup_stream() with write intent */
#define _FDEV_SETUP_RW    (__SRD|__SWR)	/**< fdev_setup_stream() with read/write intent */
#define _FDEV_SETUP_RWB   __SRWB /**< Read/write from buffers */

/**
 * \def EOF
 * \brief Definition of the End Of File condition.
 * 
 * Use with a minus sign:
 * \code{.c}
 * if(x == -EOF) {
 *     error();
 * }
 * \endcode
 */
#define EOF 1

/**
 * \brief Definition of the standard input stream.
 * \warning This stream is not initialized by default.
 * \see setup_std_streams
 */
#define stdin  (__iob[0])

/**
 * \brief Definition of the standard output stream.
 * \warning This stream is not initialized by default.
 * \see setup_std_streams
 */
#define stdout (__iob[1])

/**
 * \brief Definition of the standard error stream.
 * \warning This stream is not initialized by default.
 * \see setup_std_streams
 */
#define stderr (__iob[2])

#define fdev_setup_stream(stream, p, g, f) \
	do { \
		(stream)->put = p; \
		(stream)->get = g; \
		(stream)->flags = f; \
		(stream)->data = 0; \
	} while(0)

/**
 * \brief Define a file stream.
 * \param defname Variable name of the stream.
 * \param r Read function pointer.
 * \param w Write function pointer.
 * \param p Put function pointer.
 * \param g Get function pointer.
 * \param f Flush function pointer.
 * \param n File name function pointer.
 * \param fl Flags byte.
 * \param d Data pointer.
 * \warning The file name (<b>n</b>) MUST be unique.
 * 
 * This defines an initialized file stream structure.
 */
#define FDEV_SETUP_STREAM(defname, r, w, p, g, f, n, fl, d) \
	FILE defname = {	\
		.write = r,		\
		.read = w,		\
		.put = p,		\
		.get = g,		\
		.flush = f,		\
		.name = n,		\
		.flags = fl,	\
		.data = d,		\
		 /* default initializations */ \
		.index = 0,		\
		.length = 0,	\
		.buff = NULL,	\
	}

extern FILE *__iob[];

__DECL
extern int fputc(int c, FILE *file);
extern int fputs(char *s, FILE *f);
extern int putc(int c, FILE *f);
extern int fgetc(FILE *stream);

extern int fdputc(int c, int fd);
extern int fdgetc(int fd);

extern int write(int fd, const void *buff, size_t size);
extern int read(int fd, void *buff, size_t size);
extern void fdmode(int fd, unsigned char mode);
extern int flush(int fd);

extern int open(char *fname, unsigned char mode);
extern FILE *fdopen(int fd);
extern int close(int fd);

extern int vfprintf(FILE *stream, const char *fmt, va_list ap);

/* non-posix functions */
extern int iob_add(FILE *stream);
__DECL_END


#endif /* __STDIO_H */