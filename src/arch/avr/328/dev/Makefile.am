include ../../../../../Makefile.flags
LOCAL_LIB=@top_srcdir@/include
OPT_SCRS=

if USART
OPT_SCRS+= usart.c
endif

if SPI
OPT_SCRS+= spibus.c
endif

if IIC
OPT_SCRS+= twibus.c
endif

noinst_LTLIBRARIES = libdev.la
libdev_la_SOURCES = $(OPT_SCRS)
