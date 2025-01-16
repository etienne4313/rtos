#
# RTOS files
#
OS_LIB = $(UCOS_II)/../lib
UCOS_II_SRC = $(UCOS_II)/Source
UCOS_II_PORT = $(UCOS_II)/Ports/AVR/Generic/gcc

os_objects = $(UCOS_II_SRC)/ucos_ii.o
os_objects += $(patsubst %.c,%.o,$(wildcard $(UCOS_II_PORT)/*.c))
os_objects += $(patsubst %.c,%.o,$(wildcard $(OS_LIB)/*.c))

#
# Cross Compiler
# 	/usr/bin/avr-gcc
# 	/usr/lib/avr
#
CROSS_COMPILE = avr-
CC = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

#
# AVRDUDE
#
AVR_DUDE=/usr/bin/avrdude
AVR_DUDECONF=/usr/share/arduino/hardware/tools/avrdude.conf

ifeq ($(CPU),mega2560)
CFLAGS = -Wall -Os -DF_CPU=16000000UL -mmcu=atmega2560 -DARDUINO=10608 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -std=c99 -c -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB) -Dmega2560
LDFLAGS = -mmcu=atmega2560
FLASH = $(AVR_DUDE) -C $(AVR_DUDECONF) -patmega2560 -cwiring  -P /dev/ttyACM0 -b 115200 -D -U flash:w:$(PROG)
endif

ifeq ($(CPU),mega328)
CFLAGS = -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -std=c99 -c -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB)
LDFLAGS = -mmcu=atmega328p
FLASH = $(AVR_DUDE) -C $(AVR_DUDECONF) -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:$(PROG)
endif

ifeq ($(CPU),mega328_nano)
CFLAGS = -Wall -Os -DF_CPU=16000000UL -D_BUG_328_NANO_ -mmcu=atmega328p -std=c99 -c -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB)
LDFLAGS = -mmcu=atmega328p
FLASH = $(AVR_DUDE) -C $(AVR_DUDECONF) -F -V -c arduino -p ATMEGA328P -P  /dev/ttyUSB0 -b 57600 -U flash:w:$(PROG).hex
endif

all_objects := $(os_objects) $(common_objects)

all: $(all_objects)
	$(CC) -o $(PROG) $(all_objects) $(LDFLAGS)
	$(OBJCOPY) -O ihex -R .eeprom $(PROG) $(PROG).hex

debug:
	$(OBJDUMP) -D $(PROG) > $(PROG).diss
	$(OBJDUMP) -x $(PROG) |grep -A6 "Idx Name"
	$(OBJDUMP) -x $(PROG) |grep " .bss" |grep -v "*" |sort -nk 5  |tail -20
	$(OBJDUMP) -x $(PROG) |grep " .data" |grep -v "*" |sort -nk 5  |tail -20

clean:
	rm -f $(all_objects) $(PROG) $(PROG).hex $(PROG).diss

flash:
	$(FLASH)
