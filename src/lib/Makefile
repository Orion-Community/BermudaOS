#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../include
TARGET			= lib.o
ARDUINO_LIBS		=

CSRCS			= spiram.c
# CPPSRCS			= linkedlist.cpp

include ../../make/Makefile.$(BUILDSYS)
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
	-rm -vf $(OBJS)

