#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

AVRLIB			= /usr/lib/avr/include
LOCAL_LIB		= ../../../include
TARGET			= avr.o
ARDUINO_LIBS		=

CSRCS			= signal.c
CPPSRCS			= serial.cpp
include ../../../make/Makefile.defs

OBJS			+= arduino.o

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

arduino.o:
	$(MAKE) -C arduino/

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS) $(TARGET)
	$(MAKE) -C arduino/ clean
