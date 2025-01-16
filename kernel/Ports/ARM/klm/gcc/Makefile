MOD = /nobackup/upstream/linux_pi
CURR := $(shell pwd)
KLM := $(notdir $(PROG))
#
# RTOS files
#
OS_LIB = $(UCOS_II)/../lib
UCOS_II_SRC = $(UCOS_II)/Source
UCOS_II_PORT = $(UCOS_II)/Ports/ARM/klm/gcc

# BUG with realpath
os_objects = ../../../../Source/ucos_ii.o
os_objects += switch.o os_cpu.o klm_amp.o klm_kthread.o time.o
os_objects += ../../../../../lib/time.o ../../../../../lib/maths.o

#os_objects := $(UCOS_II_SRC)/ucos_ii.o
#os_objects += $(patsubst %.c,%.o,$(wildcard $(UCOS_II_PORT)/*.c))
#os_objects += $(patsubst %.c,%.o,$(wildcard $(OS_LIB)/*.c))
#os_objects += switch.o

#
# Kernel makefile doesn't support absolute path
#
#tmp_obj := $(shell realpath -s --relative-to $(CURR) $(common_objects))
#tmp_obj1 := $(shell realpath -s --relative-to $(CURR) $(os_objects))

obj-m += $(KLM).o
#$(KLM)-y := $(tmp_obj1) $(tmp_obj)
$(KLM)-y := $(os_objects) $(common_objects)

#
# __KTHREAD_MODE__ or __AMP_MODE__
#
ccflags-y := -I $(INCLUDE) -I $(UCOS_II_SRC) -I $(UCOS_II_PORT) -I $(OS_LIB) -D__KTHREAD_MODE__

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- V=1 -C $(MOD) M=$(CURR) modules
	mv $(KLM).ko $(PROG).ko

OBJDUMP = $(CROSS_COMPILE)objdump
debug:
	$(OBJDUMP) -D $(PROG).ko >$(PROG).diss

clean:
	make -C $(MOD) M=$(CURR) clean
	rm -f $(os_objects) $(PROG).ko $(common_objects)

