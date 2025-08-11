KERNELRELEASE := $(shell uname -r)
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

obj-m += awesome_nvme.o
awesome_nvme-objs := awesome_nvme_knl.o awesome_pci.o \
	awesome_step_init.o awesome_bar.o awesome_fops.o \
	awesome_cdev.o

.PHONY: all clean

all:
	bear --output $(CURRENT_DIR)/../.vscode/compile_commands_knl.json -- make -C /lib/modules/$(KERNELRELEASE)/build M=$(CURRENT_DIR) modules

clean:
	make -C /lib/modules/$(KERNELRELEASE)/build M=$(CURRENT_DIR) clean
