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

# CPU flags
AC_ARG_ENABLE([atmega328p],
	AS_HELP_STRING([--enable-atmega328p], [Enable build of atmega328p architecture.]),
	[enable_atmega328p=yes],
	[]
)
AC_ARG_ENABLE([atmega328],
	AS_HELP_STRING([--enable-atmega328], [Enable build of atmega328 architecture.]),
	[enable_atmega328=yes],
	[]
)

# arch flags
AC_ARG_ENABLE([avr],
	AS_HELP_STRING([--enable-avr], [Enable build of avr architecture.]),
	[arch=avr],
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
                 src/lib/Makefile
                 src/sys/Makefile
                 src/sys/events/Makefile])
AC_OUTPUT
