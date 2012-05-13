#
# Makefile definitions
# Created by Michel Megens on 12th of March 2012
#

LOCAL_LIB		= ../../include
TARGET			= sys.o
ARDUINO_LIBS		=

CSRCS			= thread.c sched.c mem.c virt_timer.c
CPPSRCS			=
ASMSRCS			=

include ../../make/Makefile.$(BUILDSYS)
include ../../make/Makefile.defs

OBJS 			+= events.o

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

events.o:
	$(MAKE) -C events/ BUILDSYS=$(BUILDSYS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	$(MAKE) -C events/ clean BUILDSYS=$(BUILDSYS)
	-rm -vf $(OBJS)
