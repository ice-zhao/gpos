CC=gcc
LD=ld
AS=as
ARCHINCLUDE=$(PWD)/arch/x86/asm/include
BOOTLOADER_INCLUDE=${PWD}/bootloader/x86/include

ASMKFLAGS	= -I$(PWD)/include/ -I$(ARCHINCLUDE) -I$(BOOTLOADER_INCLUDE)
LDFLAGS = -m elf_i386 -nolibc -nostdlib
CFLAGS = -m32 -fno-builtin -fno-stack-protector -I ${PWD}/arch/x86/include

objs-fs=buffer.o

all: $(objs-fs)

buffer.o: buffer.c
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<

clean:
	rm -rf $(objs-fs)
