include ../../../Makefile.flags
BUS=

if AVR
BUS+=bus/avr-i2c.c
endif

noinst_LTLIBRARIES=libi2c.la
libi2c_la_SOURCES=i2c-dev.c i2c-core.c $(BUS)

