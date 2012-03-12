#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../include
TARGET			= raw.elf
ARDUINO_LIBS		=

CSRCS			=
CPPSRCS			=
ARCH			= avr
DEPS			= arch.o lib.o
BUILDSYS		= windows

include ../make/Makefile.$(BUILDSYS)
include ../make/Makefile.defs


.PHONY: all clean $(TARGET)
all: $(TARGET)


$(TARGET): $(DEPS) $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(DEPS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

arch.o:
	$(MAKE) -C arch/ ARCH=$(ARCH) BUILDSYS=$(BUILDSYS)

lib.o:
	$(MAKE) -C lib/ BUILDSYS=$(BUILDSYS)

clean:
	-rm -fv $(TARGET) $(DEPS)
	$(MAKE) -C arch/ ARCH=$(ARCH) BUILDSYS=$(BUILDSYS) clean
	$(MAKE) -C lib/ cleanBUILDSYS=$(BUILDSYS)
