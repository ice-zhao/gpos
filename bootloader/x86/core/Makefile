CC=gcc
LD=ld
AS=as
BOOTLDFILE=$(PWD)/bootloader/x86/boot_x86.ld
LOADERLDFILE=$(PWD)/bootloader/x86/hdisk/hdloader_x86.ld
OBJCOPY=objcopy
HDIMAGE=$(PWD)/utils/hd.img
ARCHINCLUDE=$(PWD)/arch/x86/asm/include
BOOTLOADER_INCLUDE=${PWD}/bootloader/x86/include

ASMKFLAGS= -I include/ -I $(ARCHINCLUDE) -I $(BOOTLOADER_INCLUDE)
LDFLAGS= -m elf_i386 -nolibc -nostdlib
CFLAGS= -m32 -fno-builtin -fno-stack-protector


all: iprintk.o

iprintk.o: iprintk.S
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<



clean:
	rm -rf iprintk.o

.PHONY: clean all
