CC=gcc


all: bootloader kernel


bootloader:
	make all -C ./bootloader/x86


kernel:
	make all -C ./arch/x86

install: bootloader kernel
	make install -C ./bootloader/x86
	make install -C ./arch/x86

clean:
	make clean -C ./bootloader/x86
	make clean -C ./arch/x86

.PHONY: bootloader kernel install clean
