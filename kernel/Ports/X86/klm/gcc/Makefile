MOD = /lib/modules/$(shell uname -r)/build
CURR := $(shell pwd)
KLM := $(notdir $(PROG))

#
# RTOS files
#
OS_LIB = $(UCOS_II)/../lib
UCOS_II_SRC = $(UCOS_II)/Source
UCOS_II_PORT = $(UCOS_II)/Ports/X86/klm/gcc

os_objects := $(UCOS_II_SRC)/ucos_ii.o
os_objects += $(patsubst %.c,%.o,$(wildcard $(UCOS_II_PORT)/*.c))
os_objects += $(patsubst %.c,%.o,$(wildcard $(OS_LIB)/*.c))
os_objects += switch.o

#
# Kernel makefile doesn't support absolute path
#
tmp_obj := $(shell realpath -s --relative-to $(CURR) $(common_objects))
tmp_obj1 := $(shell realpath -s --relative-to $(CURR) $(os_objects))

obj-m += $(KLM).o
$(KLM)-y := $(tmp_obj1) $(tmp_obj)

# 
# __KTHREAD_MODE__ or __AMP_MODE__
#
ccflags-y := -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB) -D__KTHREAD_MODE__

all:
	make V=1 -C $(MOD) M=$(CURR) modules
	mv $(KLM).ko $(PROG).ko

debug:

clean:
	make -C $(MOD) M=$(CURR) clean
	rm -f $(os_objects) $(PROG).ko $(common_objects)

