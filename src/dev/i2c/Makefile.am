include ../../../Makefile.flags
noinst_LTLIBRARIES=libi2c.la
libi2c_la_SOURCES=i2c-dev.c i2c-core.c

if AVR
libi2c_la_SOURCES+=bus/avr-i2c.c
endif

LOCAL_LIB=@top_srcdir@include

