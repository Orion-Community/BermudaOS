include ../../../Makefile.flags
AR=$(AR_TOOL)
lib_LIBRARIES=libi2c.a
libi2c_a_SOURCES=i2c-dev.c i2c-core.c

if AVR
libi2c_a_SOURCES+=bus/avr-i2c.c
endif

LOCAL_LIB=../../include

