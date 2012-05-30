#
# Makefile definitions
# Created by Michel Megens on 23th of May 2012
#

LOCAL_LIB		= ../../include
TARGET			= dev.o
ARDUINO_LIBS		=

CSRCS			= devreg.c spibus.c
CPPSRCS			=
ASMSRCS			=
LIBDIR			= ../../lib

include ../../make/Makefile.$(BUILDSYS)
include ../../make/Makefile.defs

.PHONY: all clean $(TARGET)
all: $(TARGET)
	-mv -fv $(TARGET) ../

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<
	$(AR) $(ARFLAGS) $(LIBDIR)/lib$(@:.o=.a) $@

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	-rm -vf $(OBJS)
