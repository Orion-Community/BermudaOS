include ../../../Makefile.flags
DEP_FILES=

if ENABLE_ATMEGA328P
DEP_FILES+=328/328-io.c 328/serialio.c 328/328-timer.c 328/dev/spibus.c 328/dev/twibus.c 328/dev/usart.c
endif

if ENABLE_UNO
DEP_FILES+=arduino/init.c arduino/arduino-io.c
endif


lib_LIBRARIES=libavr.a
libavr_a_SOURCES=twif.c usartif.c timer.c stack.c spif.c pwm.c out.c io.c byte_order.c adc.c task_switch.S $(DEP_FILES)
LOCAL_LIB=../../../include
AR=$(AR_TOOL)
