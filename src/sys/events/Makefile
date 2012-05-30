#
# Makefile definitions
# Created by Michel Megens on 9th of May 2012
#

LOCAL_LIB               = ../../../include
LIBDIR			= ../../../lib
TARGET                  = events.o
ARDUINO_LIBS            =

CSRCS			= event.c
CPPSRCS                 =
ASMSRCS                 =

include ../../../make/Makefile.$(BUILDSYS)
include ../../../make/Makefile.defs

.PHONY: all clean $(TARGET)
all: $(TARGET)
	$(AR) $(ARFLAGS) $(LIBDIR)/libEvents.a $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS)
