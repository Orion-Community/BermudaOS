include ../../../Makefile.flags
BUS=

if ATMEGA
BUS+=busses/atmega-pwm.c
endif

noinst_LTLIBRARIES=libpwm.la
libpwm_la_SOURCES=pwm-dev.c pwm-core.c $(BUS)
