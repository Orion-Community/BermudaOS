include ../../../Makefile.flags
BUS=

if ATMEGA
BUS+=busses/atmega-spi.c
endif

noinst_LTLIBRARIES=libspi.la
libspi_la_SOURCES=spi-dev.c spi-core.c $(BUS)
