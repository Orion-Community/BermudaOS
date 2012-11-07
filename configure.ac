#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([BermudaNet], [0.0.1], [Web: http://bugs.michelmegens.net :: E-Mail: report@bugs.michelmegens.net])
AM_INIT_AUTOMAKE(BermudaNet, 0.0.1)
LT_INIT([disable-shared,static])
AC_CONFIG_HEADERS([netconfig.h])
AC_CONFIG_MACRO_DIR([m4])

# Set build variables.
CFLAGS=
CXXFLAGS=
LDFLAGS=
ASFLAGS=
CCASFLAGS=

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Add definitions
AC_ARG_ENABLE([avr],
              AS_HELP_STRING([--enable-avr], [Enables the AVR architecture]),
              [enable_avr=yes],
              []
)
AC_ARG_VAR(CPU, [CPU type which should be passed to -mmcu])
AC_ARG_VAR(OS_INCLUDE, [Absolute path to the include directory of Bermuda/OS])
AC_ARG_VAR(RX_QU_LENGTH, [Maximum length of receive queues])
AC_ARG_VAR(TX_QU_LENGTH, [Maximum length of transmit queues])

# Set default values
if test "x$RX_QU_LENGTH" = "x"; then
RX_QU_LENGTH=100
fi

if test "x$TX_QU_LENGTH" = "x"; then
TX_QU_LENGTH=100
fi

AM_CONDITIONAL(AVR, test x$enable_avr = xyes)
# Checks for libraries.

# Checks for header files.
AH_TOP([#ifndef __BERMUDANET_CONF_H])
AH_TOP([#define __BERMUDANET_CONF_H])
AH_BOTTOM([#define BERMUDANET_CONFIG 1])
AH_BOTTOM([#include <bermudanet.h>])
AH_BOTTOM([#endif])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/core/Makefile]
)
AC_OUTPUT
