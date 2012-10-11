include ../../../Makefile.flags
BUS=

if ATMEGA
BUS+=busses/atmega-usart.c
endif

noinst_LTLIBRARIES=libusart.la
libusart_la_SOURCES=usart-dev.c $(BUS)

