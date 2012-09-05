LOCAL_LIB		= ../include
TARGET			= app.hex
ARDUINO_LIBS		=

CSRCS			= main.c
CPPSRCS			= 
LIBDIR			= ../lib
LIBS			= -lBermuda -ltwidev -ldevreg -lspibus -lEvents -l24c02 \
			  -lspiram
BUILDSYS		= windows

include ../make/Makefile.$(BUILDSYS)
include ../make/Makefile.defs

.PHONY: all clean upload $(TARGET)

all: $(TARGET) $(OBJS)
	$(CC) -mmcu=$(MCU) -lm -Wl,--gc-sections -Os -o temp.elf $(OBJS) -L$(LIBDIR) $(LIBS)
	$(OBJCOPY) -R .eeprom -O ihex temp.elf $(TARGET)
#	-rm -fv temp.elf

%o: %c
	$(CC) $(CCFLAGS) -c -o $@ $<

upload: $(TARGET)
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) -U flash:w:$(TARGET):i

clean:
	-rm -fv $(OBJS)
	-rm -fv $(TARGET)
	-rm -fv temp.elf
