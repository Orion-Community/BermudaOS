include ../../../../Makefile.flags
LOCAL_LIB=../../../../include
AR=$(AR_TOOL)

SUBDIRS=dev
noinst_LTLIBRARIES = lib328.la
lib328_la_SOURCES=328-io.c 328-timer.c serialio.c
lib328_la_LIBADD=dev/libdev.la
