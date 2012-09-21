SUBDIRS=i2c
lib_LTLIBRARIES=libdev.la
libdev_la_SOURCES=devreg.c spibus.c twidev.c
libdev_la_LIBADD=i2c/libi2c.la
LOCAL_LIB=@top_srcdir@/include
include ../../Makefile.flags
