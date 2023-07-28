MODULE_NAME = tcdtne

obj-m	:= $(MODULE_NAME).o

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

all: module

.PHONY: all module clean unload load

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

unload:
	if lsmod | awk -v target="$(MODULE_NAME)" 'NR > 1 && $$1 == target { found=1; exit } END { exit !found }'; then \
		sudo rmmod $(MODULE_NAME); \
	fi

load: module unload
	sudo insmod $(MODULE_NAME).ko
	sudo chmod a+r /dev/$(MODULE_NAME)
