#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE(BermudaOS, 0.0.5)
#AC_CONFIG_SRCDIR([app/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX([avr-g++])
AC_PROG_CC([avr-gcc])
AM_PROG_AS([avr-gcc])

CFLAGS=
CXXFLAGS=
LDFLAGS=

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lBermuda':
#AC_CHECK_LIB([Bermuda], [main])
# FIXME: Replace `main' with a function in `-lEvents':
#AC_CHECK_LIB([Events], [main])
# FIXME: Replace `main' with a function in `-ldevreg':
#AC_CHECK_LIB([devreg], [main])
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lspibus':
#AC_CHECK_LIB([spibus], [main])
# FIXME: Replace `main' with a function in `-lspiram':
#AC_CHECK_LIB([spiram], [main])
# FIXME: Replace `main' with a function in `-ltwidev':
#AC_CHECK_LIB([twidev], [main])

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
                 src/arch/avr/328/Makefile
                 src/arch/avr/328/dev/Makefile
                 src/arch/avr/Makefile
                 src/arch/avr/arduino/Makefile
                 src/dev/Makefile
                 src/lib/Makefile
                 src/sys/Makefile
                 src/sys/events/Makefile])
AC_OUTPUT
