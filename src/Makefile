#
# Makefile definitions
# Created by Michel Megens on 9th of March 2012
#

LOCAL_LIB		= ../../../include
RAW_TARGET		= raw.elf
TARGET			= bermuda.hex
ARDUINO_LIBS		=
AVRDUDE_ARD_BAUDRATE    = 115200
AVRDUDE_ARD_PROGRAMMER  = arduino

CSRCS			=
CPPSRCS			=
ARCH			= avr
DEPS			= arch.o lib.o sys.o
BUILDSYS		= linux 

include ../make/Makefile.$(BUILDSYS)
include ../make/Makefile.defs


.PHONY: all clean $(TARGET) upload
all: $(TARGET)


$(RAW_TARGET): $(DEPS) $(OBJS)
	$(CC) -mmcu=$(MCU) -lm -Wl,--gc-sections -Os -o $(RAW_TARGET) $(DEPS)

$(TARGET): $(RAW_TARGET)
	$(STRIP) --strip-all $(RAW_TARGET)
	$(OBJCOPY) -R .eeprom -O ihex $(RAW_TARGET) $(TARGET)

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

upload: $(TARGET)
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) -U flash:w:$(TARGET):i

doc:
	$(DOXY) -DSx ../doc/Doxy

clean:

	-rm -fv $(TARGET) $(DEPS) $(RAW_TARGET)
	-rm -Rfv ../doc/html/
	$(MAKE) -C arch/ clean ARCH=$(ARCH) BUILDSYS=$(BUILDSYS)
	$(MAKE) -C lib/ clean BUILDSYS=$(BUILDSYS)
	$(MAKE) -C sys/ clean BUILDSYS=$(BUILDSYS)

