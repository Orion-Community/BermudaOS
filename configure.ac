#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([BermudaOS], [0.0.5], [Web: bugs.michelmegens.net :: Mail: report@bugs.michelmegens.net])
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

# CPU flags
AC_ARG_ENABLE([atmega328p],
	AS_HELP_STRING([--enable-atmega328p], [Enable build of atmega328p architecture.]),
	[enable_atmega328p=yes arch=avr atmega=yes],
	[]
)
AC_ARG_ENABLE([atmega328],
	AS_HELP_STRING([--enable-atmega328], [Enable build of atmega328 architecture.]),
	[enable_atmega328=yes],
	[]
)

AC_ARG_ENABLE([arm],
	AS_HELP_STRING([--enable-arm], [Enable build of arm architecture.]),
	[arch=arm-linux-gnueabi],
	[]
)

#board flags
AC_ARG_ENABLE([uno],
	AS_HELP_STRING([--enable-uno], [Enable build of arduino uno support.]),
	[enable_uno=yes],
	[]
)

AC_ARG_VAR(EXTRAM, [Amount of !EXTENDED! RAM available.])
AC_ARG_VAR(IDLE_STACK, [Size of the idle stack.])
AC_ARG_VAR(MAIN_STACK, [Size of the main stack.])

# code enable flags
AC_ARG_ENABLE([threads],
	AS_HELP_STRING([--disable-threads], [Disable the support of threads.]),
	[threads=no],
	[threads=yes] # standard enabled
)

AC_ARG_ENABLE([i2c],
	AS_HELP_STRING([--disable-i2c], [Disable the support of I2C.]),
	[iic=no],
	[iic=yes]
)

AC_ARG_ENABLE([pwm],
	AS_HELP_STRING([--disable-pwm], [Disable the support of PWM.]),
	[pwm=no],
	[pwm=yes]
)

AC_ARG_ENABLE([spi],
	AS_HELP_STRING([--disable-spi], [Disable the support of SPI.]),
	[spi=no],
	[spi=yes]
)

AC_ARG_ENABLE([usart],
	AS_HELP_STRING([--disable-usart], [Disable the support of the USART.]),
	[usart=no],
	[usart=yes]
)

AC_ARG_ENABLE([adc],
	AS_HELP_STRING([--disable-adc], [Disable the support of the ADC.]),
	[adc=no],
	[adc=yes]
)

# -------------------------------------------------------
# CPU conditionals
# -------------------------------------------------------
AM_CONDITIONAL(ENABLE_ATMEGA328P, test x$enable_atmega328p = xyes)
AM_CONDITIONAL(ENABLE_ATMEGA328, test x$enable_atmega328 = xyes)

# -------------------------------------------------------
# Board conditionals
# -------------------------------------------------------
AM_CONDITIONAL(ENABLE_UNO, test x$enable_uno = xyes)

# -------------------------------------------------------
# Arch conditionals
# -------------------------------------------------------
AM_CONDITIONAL(AVR, test x$arch = xavr)
AM_CONDITIONAL(ARM, test x$arch = xarm-linux-gnueabi)
AM_CONDITIONAL(ATMEGA, test x$atmega = xyes)

# -------------------------------------------------------
# Code enablers
# -------------------------------------------------------
AM_CONDITIONAL(THREADS, test x$threads = xyes)
AM_CONDITIONAL(IIC, test x$iic = xyes)
AM_CONDITIONAL(SPI, test x$spi = xyes)
AM_CONDITIONAL(USART, test x$usart = xyes)
AM_CONDITIONAL(ADC, test x$adc = xyes)
AM_CONDITIONAL(PWM, test x$pwm = xyes)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

#extram
if test "x$EXTRAM" = "x"; then
EXTRAM=0
fi

if test "x$IDLE_STACK" = "x"; then
IDLE_STACK=100
fi

if test "x$MAIN_STACK" = "x"; then
MAIN_STACK=128
fi

# autoheader defines
if test "x$iic" = "xyes"; then
AC_DEFINE([__I2C__], [1], [Defines wether I2C is enabled])
AC_DEFINE([__TWI__], [1], [Defines wether I2C is enabled])
fi

if test "x$iic" = "xyes"; then
AC_DEFINE([__SPI__], [1], [Defines wether SPI is enabled])
fi

if test "x$usart" = "xyes"; then
AC_DEFINE([__USART__], [1], [Defines wether USART is enabled])
fi

if test "x$adc" = "xyes"; then
AC_DEFINE([__ADC__], [1], [Defines wether ADC is enabled])
fi

if test "x$threads" = "xyes"; then
AC_DEFINE([__THREADS__], [1], [Defines wether threads are enabled])
AC_DEFINE([__EVENTS__], [1], [Defines wether events are enabled])
fi

if test "x$pwm" = "xyes"; then
AC_DEFINE([__PWM__], [1], [Defines wether PWM's are enabled.])
fi

if test "x$atmega" = "xyes"; then
AC_DEFINE([ATMEGA], [1], [Defines weather we are using an atmega based CPU])
fi

AH_BOTTOM([#include <configmake.h>])

# Checks for header files.
AC_CHECK_HEADERS([stdarg.h])

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
                 src/dev/pwm/Makefile
                 src/dev/usart/Makefile
                 src/lib/Makefile
                 src/lib/c/Makefile
                 src/lib/c/stdlib/Makefile
                 src/lib/c/stdio/Makefile
                 src/lib/c/string/Makefile
                 src/sys/Makefile
                 src/sys/events/Makefile
                 src/fs/Makefile])
AC_OUTPUT
