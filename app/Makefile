LOCAL_LIB		= ../include
TARGET			= app.hex
ARDUINO_LIBS		=

CSRCS			= main.c
CPPSRCS			= 
LIBDIR			= ../build/install/lib
LIBEXEC			= ../build/install/libexec
LIBS			= -lavr -ldev -lsys -lavr -l24c02 -lspiram -lcbermuda -lvfs -ldev
BUILDSYS		= linux
TOP_SRCDIR		= ../build

include ../make/Makefile.$(BUILDSYS)
include ../make/Makefile.defs

.PHONY: all clean upload $(TARGET)

all: $(TARGET) $(OBJS)
	$(CC) -mmcu=$(MCU) -nostdlib -nostdinc -Wl,--gc-sections -Os -o temp.elf \
	$(LIBEXEC)/boot.elf $(OBJS) -L$(LIBDIR) $(LIBS) -lgcc
	$(OBJCOPY) -R .eeprom -O ihex temp.elf $(TARGET)
#	-rm -fv temp.elf

%o: %c
	$(CC) $(CCFLAGS) -DHAVE_CONFIG -I$(TOP_SRCDIR) -c -o $@ $<

upload: $(TARGET)
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) -U flash:w:$(TARGET):i

clean:
	-rm -fv $(OBJS)
	-rm -fv $(TARGET)
	-rm -fv temp.elf
