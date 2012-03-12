#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../include
TARGET			= arch.o
ARDUINO_LIBS		=

CSRCS			=
CPPSRCS			=

include ../../make/Makefile.$(BUILDSYS)
include ../../make/Makefile.defs


.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(ARCH).o $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(ARCH).o

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

$(ARCH).o:
	$(MAKE) -C $(ARCH)/ BUILDSYS=$(BUILDSYS)

clean:
	-rm -fv $(ARCH).o $(OBJS)
	$(MAKE) -C $(ARCH)/ clean BUILDSYS=$(BUILDSYS)
