#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../../include
TARGET			= arduino.o
ARDUINO_LIBS		=

CSRCS			= init.c io.c
CPPSRCS			=

include ../../../../make/Makefile.$(BUILDSYS)
include ../../../../make/Makefile.defs

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -vf $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS) $(TARGET)
