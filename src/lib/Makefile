#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

AVRLIB			= /usr/lib/avr/include
LOCAL_LIB		= ../../include
TARGET			= lib.o
ARDUINO_LIBS		=

CSRCS			= xorlist.c
CPPSRCS			= out.cpp
include ../../make/Makefile.defs

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	@echo $(OBJS)

