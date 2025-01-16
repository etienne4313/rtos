#
# RTOS files
#
OS_LIB = $(UCOS_II)/../lib
UCOS_II_SRC = $(UCOS_II)Source
UCOS_II_PORT = $(UCOS_II)Ports/X86/user/gcc

os_objects := $(UCOS_II_SRC)/ucos_ii.o
os_objects += $(patsubst %.c,%.o,$(wildcard $(UCOS_II_PORT)/*.c))
os_objects += $(patsubst %.c,%.o,$(wildcard $(OS_LIB)/*.c))
asm_objects := switch.S

CFLAGS = -c -Wall -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB)
LDFLAGS = -lrt -lpthread

all_objects := $(os_objects) $(common_objects)

all: $(all_objects)
	as $(asm_objects) -o /tmp/switch.out
	$(CC) -o $(PROG) $(all_objects) /tmp/switch.out $(LDFLAGS)

debug:
	$(OBJDUMP) -D $(PROG) >$(PROG).diss

clean:
	rm -f $(all_objects) $(PROG) $(PROG).hex $(PROG).diss

