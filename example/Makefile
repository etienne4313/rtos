PWD := $(shell pwd)
UCOS_II=~/rtos/kernel/
include .config

#
# Build all the subdir
#
PROGRAM := simple_example ping_pong_sem simple_sem timer_wheel_test timing_test

export UCOS_II KDIR CPU INSTALL

all:
	for n in $(PROGRAM); do $(MAKE) -C $$n || exit 1; done

install:
	mkdir -p $(INSTALL)
	for n in $(PROGRAM); do $(MAKE) -C $$n install; done

debug:
	for n in $(PROGRAM); do $(MAKE) -C $$n debug; done

clean:
	for n in $(PROGRAM); do $(MAKE) -C $$n clean; done

flash:
	for n in $(FLASH_PROGRAM); do $(MAKE) -C $$n flash; done

mrproper:
	find . -name ".*.cmd" -type f -print0 | xargs -0 /bin/rm -f
	find . -name "*.ko" -type f -print0 | xargs -0 /bin/rm -f
	find . -name "*.o" -type f -print0 | xargs -0 /bin/rm -f
	rm -rf $(INSTALL)
	rm -f .config

