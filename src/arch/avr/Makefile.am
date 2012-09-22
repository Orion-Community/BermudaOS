include ../../../Makefile.flags
DEP_FILES=

if ENABLE_ATMEGA328P
MAYBE_MCU=328
endif

if ENABLE_ATMEGA328
MAYBE_MCU=328
endif

if ENABLE_UNO
MAYBE_ARD=arduino
endif

SUBDIRS=$(MAYBE_MCU) $(MAYBE_ARD)

lib_LTLIBRARIES=libavr.la
libavr_la_SOURCES=twif.c usartif.c timer.c stack.c spif.c pwm.c out.c io.c byte_order.c adc.c task_switch.S $(DEP_FILES)
libavr_la_LIBADD=328/lib328.la arduino/libarduino.la
libavr_la_LIBTOOLFLAGS=--tag=CC
LOCAL_LIB=../../../include
