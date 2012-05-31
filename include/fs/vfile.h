/*
 *  BermudaOS - Device drivers
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

#ifndef __VFILE_H_
#define __VFILE_H_

#include <bermuda.h>

//! \file fs/vfile.h

/**
 * \struct _vfile
 * \brief Virtual file structure.
 * \note Usualy associated with a device.
 */
struct _vfile
{
        unsigned char mode; //!< File mode.
        
        /**
         * \brief Close the file.
         * \param f File to close.
         */
        int (*close)(struct _vfile *f);
        
        /**
         * \brief Flush the file.
         * \param f File to flush.
         * 
         * Ensures that all bytes which are written to the file using 'write' are
         * flushed to to the block device.
         */
        int (*flush)(struct _vfile *f);
        
        /**
         * \brief Write to the file.
         * \param f File to write to.
         * \param buf Data which has to be written.
         * \param len Length of <b>buf</b>.
         * \see flush
         * 
         * Writes the given data byte by byte (until <b>len</b> is reached to the
         * file (driver). A flush might be necessary.
         */
        int (*write)(struct _vfile *f, const void *buf, size_t len);
        
        /**
         * \brief File read.
         * \param f File to read from.
         * \param buf Buffer to store data in.
         * \param len Amount of bytes to read.
         * 
         * Reads data from the file until an EOF is detected or <b>len</b> is
         * reached.
         */
        int (*read) (struct _vfile *f, void *buf, size_t len);
        
        /**
         * \brief File data pointer.
         * \see _device
         * 
         * Pointer to data associated with this file. In most cases it points to
         * a device structure.
         */
        void *data;
};

/**
 * \typedef VFILE
 * \brief Virtual file type.
 * \see _vfile
 */
typedef struct _vfile VFILE;

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* __VFILE_H_ */