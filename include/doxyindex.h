/*
 *  BermudaOS - Doxygen main page
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

#if !defined(__DOXYGEN__)
#       error File should not be included
#endif

/** \file doxyindex.h */

/**
 * \mainpage Home
 * \section intro Introduction
 * 
 * BermudaOS an embedded scalable, modular operating system and API. It comes
 * with: \n \n
 * * Dynamic memory management; \n
 * * Thread management;\n
 * * Thread synchronisation principles;\n
 * * Hardware and software I2C;\n
 * * Hardware and software SPI;\n
 * * RS232 (often used for text output to the screen).\n
 * * BermudaNet - See <a href="group__net.html">here</a>.
 * \n
 * The schedule algorithm implemented by BermudaOS, is a cooperative single queue
 * priority based algorithm.
 * 
 * \section source Source (download)
 * 
 * Source is available via git. 
 * <a href="http://git.michelmegens.net/viewgit/?a=summary&p=BermudaOS">Here</a>
 * are source tarballs available. Please use the latest release tag for the most
 * stable version. A git-clone can be performed from <a href="#">
 * git://git.michelmegens.net/BermudaOS.git</a>.
 * 
 * \section build Building
 * Building BermudaOS from source is done using the GNU autotools collection in combination with
 * the GNU CC for your target. Sources must first be configured using the configure script. This
 * script ships with a help option:
 * \verbatim ./configure --help \endverbatim
 * If you do not want to clobber the sources with configure output you can build in a seperate build
 * directory (execute in BermudaOS root dir):
\verbatim
mkdir build
cd build
../configure ...
\endverbatim
 */
