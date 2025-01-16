PWD := $(shell pwd)
PROG := $(PWD)/$(shell basename `pwd`)

#
# Project files
#
INCLUDE = $(PWD)
common_objects := $(patsubst %.c,../../../../../../../../../%.o,$(wildcard $(PWD)/*.c))
#PI KLM build doesn't accept realpath ??
#common_objects := $(patsubst %.c,%.o,$(wildcard $(PWD)/*.c))

export PWD PROG INCLUDE common_objects

all:
	$(MAKE) -C $(KDIR)

install:
	cp $(PROG)* $(INSTALL)

debug:
	$(MAKE) -C $(KDIR) debug

clean:
	$(MAKE) -C $(KDIR) clean

flash:
	$(MAKE) -C $(KDIR) flash

