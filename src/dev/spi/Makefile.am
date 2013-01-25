include ../../../Makefile.flags
BUS=

if ATMEGA
BUS+=busses/atmega-spi.c
endif

noinst_LTLIBRARIES=libi2c.la
libi2c_la_SOURCES=spi-dev.c $(BUS)
