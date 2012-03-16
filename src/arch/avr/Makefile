#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../include
TARGET			= avr.o
ARDUINO_LIBS		=

CSRCS			= signal.c io.c
CPPSRCS			=
ASMSRCS			= task_switch.S

include ../../../make/Makefile.$(BUILDSYS)
include ../../../make/Makefile.defs

OBJS			+= arduino.o 328.o

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

arduino.o:
	$(MAKE) -C arduino/ BUILDSYS=$(BUILDSYS)

328.o:
	$(MAKE) -C 328/ BUILDSYS=$(BUILDSYS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

%o: %S
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS) $(TARGET)
	$(MAKE) -C arduino/ clean BUILDSYS=$(BUILDSYS)
	$(MAKE) -C 328/ clean BUILDSYS=$(BUILDSYS)
