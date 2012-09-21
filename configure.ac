#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE(BermudaOS, 0.0.5)
LT_INIT([disable-shared,static])
#AC_CONFIG_SRCDIR([app/main.c])
AC_CONFIG_HEADERS([config.h])

CFLAGS=
CXXFLAGS=
LDFLAGS=
ASFLAGS=
CCASFLAGS=

# Optional flags
AC_ARG_ENABLE([atmega328p],
	AS_HELP_STRING([--enable-atmega328p], [Enable build of atmega328p architecture.]),
	[enable_atmega328p=yes],
	[]
)

AC_ARG_ENABLE([avr],
	AS_HELP_STRING([--enable-avr], [Enable build of avr architecture.]),
	[avr=yes],
	[]
)

AC_ARG_ENABLE([uno],
	AS_HELP_STRING([--enable-uno], [Enable build of arduino uno support.]),
	[enable_uno=yes],
	[]
)

AM_CONDITIONAL(ENABLE_ATMEGA328P, test x$enable_atmega328p = xyes)
AM_CONDITIONAL(ENABLE_UNO, test x$enable_uno = xyes)
AM_CONDITIONAL(AVR, test x$avr = xyes)

# Checks for programs.
if test x$avr = xyes; then
	AC_PROG_CXX([avr-g++])
	AC_PROG_CC([avr-gcc])
	AC_PROG_RANLIB([avr-ranlib])
	AM_PROG_AS([avr-gcc])
	RANLIB=avr-ranlib
	CC=avr-gcc
	CCAS=avr-gcc
else
	AC_PROG_CXX
	AC_PROG_CC
	AC_PROG_RANLIB
	AM_PROG_AS

	RANLIB=ranlib
fi

AC_PROG_LIBTOOL

# Enable enable object specific options
AM_PROG_CC_C_O

# Checks for header files.
#AC_CHECK_HEADERS([netinet/in.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/arch/Makefile
                 src/arch/avr/Makefile
                 src/arch/avr/arduino/Makefile
                 src/arch/avr/328/Makefile
                 src/arch/avr/328/dev/Makefile
                 src/dev/Makefile
                 src/dev/i2c/Makefile
                 src/lib/Makefile
                 src/sys/Makefile
                 src/sys/events/Makefile])
AC_OUTPUT
