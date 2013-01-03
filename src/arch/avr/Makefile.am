include ../../../Makefile.flags
OPT_SCRS=

if ENABLE_ATMEGA328P
MAYBE_MCU=328
endif

if ENABLE_ATMEGA328
MAYBE_MCU=328
endif

if ENABLE_UNO
MAYBE_ARD=arduino
endif

if SPI
OPT_SCRS+= spif.c
endif

if ADC
OPT_SCRS+= adc.c
endif

if THREADS
OPT_SCRS+= task_switch.S stack.c
endif

if PWM
OPT_SCRS+= pwm.c
endif

SUBDIRS=$(MAYBE_MCU) $(MAYBE_ARD)
bermudaosdir=@libdir@/bermudaos
bermudaos_LTLIBRARIES=libavr.la
libavr_la_SOURCES= timer.c io.c byte_order.c $(OPT_SCRS)
libavr_la_LIBADD=328/lib328.la arduino/libarduino.la
libavr_la_LIBTOOLFLAGS=--tag=CC --silent

