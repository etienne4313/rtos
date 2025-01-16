#
# RTOS files
#
OS_LIB = $(UCOS_II)/../lib
UCOS_II_SRC = $(UCOS_II)Source
UCOS_II_PORT = $(UCOS_II)Ports/ARM/user/gcc

os_objects = $(UCOS_II_SRC)/ucos_ii.o
os_objects += $(patsubst %.c,%.o,$(wildcard $(UCOS_II_PORT)/*.c))
os_objects += $(patsubst %.c,%.o,$(wildcard $(OS_LIB)/*.c))
asm_objects := switch.S

#
# Cross Compiler
#
CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -mabi=aapcs-linux -marm -c -Wall -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB)
LDFLAGS = -lrt -lpthread

all_objects := $(os_objects) $(common_objects)

all: $(all_objects)
	$(AS) $(asm_objects) -o /tmp/switch.out
	$(CC) -o $(PROG) $(all_objects) /tmp/switch.out $(LDFLAGS)

debug:
	$(OBJDUMP) -D $(PROG) >$(PROG).diss

clean:
	rm -f $(all_objects) $(PROG) $(PROG).hex $(PROG).diss

