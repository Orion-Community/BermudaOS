OPT_SCRS=

if IIC
MAYBE_IIC=i2c
IIC_LIB=i2c/libi2c.la
# OPT_SCRS+= twidev.c
endif

if SPI
OPT_SCRS+= spibus.c
endif

if USART
MAYBE_USART=usart
USART_LIB=usart/libusart.la
endif


SUBDIRS=$(MAYBE_IIC) $(MAYBE_USART)
lib_LTLIBRARIES=libdev.la
libdev_la_SOURCES=devreg.c $(OPT_SCRS)
libdev_la_LIBADD=$(IIC_LIB) $(USART_LIB)
include ../../Makefile.flags
