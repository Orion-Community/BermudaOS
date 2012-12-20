include ../../../Makefile.flags
BUS=

if ATMEGA
BUS+=busses/atmega-i2c.c
endif

noinst_LTLIBRARIES=libi2c.la
libi2c_la_SOURCES=i2c-dev.c i2c-core.c i2c-msg.c $(BUS)
dist_data_DATA=i2c-core-priv.h busses/atmega_priv.h

