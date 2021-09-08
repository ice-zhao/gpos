CC=gcc


all: bootloader kernel


bootloader:
	make all -C ./bootloader/x86


kernel:
	make all -C ./arch/x86

install: install_bootloader install_kernel

install_kernel: kernel
	make install -C ./arch/x86

install_bootloader: bootloader
	make install -C ./bootloader/x86


clean:
	make clean -C ./bootloader/x86
	make clean -C ./arch/x86

.PHONY: bootloader kernel install clean install_kernel install_bootloader
