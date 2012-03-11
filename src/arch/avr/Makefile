#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../include
TARGET			= avr.o
ARDUINO_LIBS		=

CSRCS			= signal.c
CPPSRCS			= serial.cpp

include ../../../make/Makefile.linux
include ../../../make/Makefile.defs

OBJS			+= arduino.o chip.o

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

arduino.o:
	$(MAKE) -C arduino/

chip.o:
	$(MAKE) -C chip/

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS) $(TARGET)
	$(MAKE) -C arduino/ clean
	$(MAKE) -C chip/ clean
