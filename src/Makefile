#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../include
LIBDIR			= ../lib
RAW_TARGET		= raw.elf
TARGET			= Bermuda
ARDUINO_LIBS	=

CSRCS			=
CPPSRCS			=
ARCH			= avr
DEPS			= arch.o lib.o sys.o dev.o
BUILDSYS		= linux

include ../make/Makefile.$(BUILDSYS)
include ../make/Makefile.defs


.PHONY: all clean doc $(TARGET) upload
all: $(DEPS)
	$(AR) $(ARFLAGS) $(LIBDIR)/lib$(TARGET).a arch.o sys.o

# $(RAW_TARGET): $(DEPS) $(OBJS)
# 	$(CC) -mmcu=$(MCU) -lm -Wl,--gc-sections -Os -o $(RAW_TARGET) $(DEPS)
# 
# $(TARGET): $(RAW_TARGET)
# 	$(STRIP) --strip-all $(RAW_TARGET)
# 	$(OBJCOPY) -R .eeprom -O ihex $(RAW_TARGET) $(TARGET)

%o: %c
	$(CC) $(CCFLAGS) -o $@ $<

%o: %cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

arch.o:
	$(MAKE) -C arch/ ARCH=$(ARCH) BUILDSYS=$(BUILDSYS)

lib.o:
	$(MAKE) -C lib/ BUILDSYS=$(BUILDSYS)

sys.o:
	$(MAKE) -C sys/ BUILDSYS=$(BUILDSYS)

dev.o:
	$(MAKE) -C dev/ BUILDSYS=$(BUILDSYS)

upload: $(TARGET)
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) -U flash:w:$(TARGET):i

clean:
	-rm -fv $(TARGET) $(DEPS) $(RAW_TARGET)
	-rm -Rfv ../doc/html/
	$(MAKE) -C arch/ clean ARCH=$(ARCH) BUILDSYS=$(BUILDSYS)
	$(MAKE) -C lib/ clean BUILDSYS=$(BUILDSYS)
	$(MAKE) -C sys/ clean BUILDSYS=$(BUILDSYS)
	$(MAKE) -C dev/ clean BUILDSYS=$(BUILDSYS)

doc:
	-doxygen ../Doxyfile
