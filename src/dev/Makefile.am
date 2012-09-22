if IIC
MAYBE_IIC=i2c
IIC_LIB=i2c/libi2c.la
endif

SUBDIRS=$(MAYBE_IIC)
lib_LTLIBRARIES=libdev.la
libdev_la_SOURCES=devreg.c spibus.c twidev.c
libdev_la_LIBADD=$(IIC_LIB)
include ../../Makefile.flags
