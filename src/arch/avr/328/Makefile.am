include ../../../../Makefile.flags
OPT_SCRS=

if USART
OPT_SCRS+= serialio.c
endif

SUBDIRS=dev
noinst_LTLIBRARIES = lib328.la
lib328_la_SOURCES=328-io.c 328-timer.c $(OPT_SCRS)
lib328_la_LIBADD=dev/libdev.la
