OPT_SCRS=

if IIC
MAYBE_IIC=i2c
IIC_LIB=i2c/libi2c.la
# OPT_SCRS+= twidev.c
endif

if SPI
OPT_SCRS+= spibus.c
endif


SUBDIRS=$(MAYBE_IIC)
lib_LTLIBRARIES=libdev.la
libdev_la_SOURCES=devreg.c $(OPT_SCRS)
libdev_la_LIBADD=$(IIC_LIB)
include ../../Makefile.flags
