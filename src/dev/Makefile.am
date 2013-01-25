OPT_SCRS=

if IIC
MAYBE_IIC=i2c
IIC_LIB=i2c/libi2c.la
# OPT_SCRS+= twidev.c
endif

if SPI
OPT_SCRS+= spibus.c
MAYBE_SPI=spi
endif

if USART
MAYBE_USART=usart
USART_LIB=usart/libusart.la
endif

if PWM
MAYBYE_PWM=pwm
PWM_LIB=pwm/libpwm.la
endif


SUBDIRS=$(MAYBE_IIC) $(MAYBYE_PWM) $(MAYBE_USART) $(MAYBE_SPI)

bermudaosdir=@libdir@/bermudaos
bermudaos_LTLIBRARIES=libdev.la
libdev_la_SOURCES=devreg.c $(OPT_SCRS)
libdev_la_LIBADD=$(IIC_LIB) $(PWM_LIB) $(USART_LIB)
include ../../Makefile.flags
