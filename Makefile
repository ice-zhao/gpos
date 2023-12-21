CC := gcc
ARCH := x86
DEST := arch/$(ARCH)
BASE := $(PWD)

export BASE DEST ARCH CC

include arch/$(ARCH)/asm/include/Makefile

all: bootloader kernel

bootloader:
	$(Q)make all -C ./bootloader/x86

#install hard disk bootloader
install_hd_bootloader:
	$(Q)make bootloader
	$(Q)make install -C ./bootloader/x86/hdisk

#only clean hard disk bootloader
clean_hd_bootloader:
	$(Q)make clean -C ./bootloader/x86/hdisk

clean_bootloader:
	$(Q)make clean -C ./bootloader/x86

kernel:
	make all -C ./lib
	make all -C ./kernel
	make all -C ./mm
	make all -C ./drivers
	make all -C ./fs
	make all -C ./arch/x86

install: install_bootloader install_kernel

install_kernel: kernel
	make install -C ./arch/x86

#install floppy bootloader
install_bootloader: bootloader
	make install -C ./bootloader/x86


clean:
	make clean -C ./mm
	make clean -C ./kernel
	make clean -C ./arch/x86
	make clean -C ./lib
	make clean -C ./drivers
	make clean -C ./fs

.PHONY: bootloader kernel install clean install_kernel install_bootloader
