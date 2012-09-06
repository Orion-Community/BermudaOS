#
# Makefile definitions
# Created by Michel Megens on 23th of May 2012
#

LOCAL_LIB		= ../../../../../include
TARGET			= dev.o
ARDUINO_LIBS		=

CSRCS			= spibus.c twibus.c usart.c
CPPSRCS			=
ASMSRCS			=
LIBDIR			= ../../../../../lib

include ../../../../../make/Makefile.$(BUILDSYS)
include ../../../../../make/Makefile.defs

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)
#	$(AR) $(ARFLAGS) $(LIBDIR)/lib$(TARGET:.o=.a) $(OBJS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS)
