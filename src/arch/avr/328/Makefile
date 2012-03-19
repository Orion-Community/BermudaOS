#
# Makefile definitions
# Created by Michel Megens on 14th of March 2012
#

LOCAL_LIB		= ../../../../include
TARGET			= 328.o
ARDUINO_LIBS		=

CSRCS			= io.c timer.c
CPPSRCS			=

include ../../../../make/Makefile.$(BUILDSYS)
include ../../../../make/Makefile.defs

OBJS                    += chip.o

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -vf $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

chip.o:
	$(MAKE) -C chip/ BUILDSYS=$(BUILDSYS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS) $(TARGET)
	$(MAKE) -C chip/ clean
