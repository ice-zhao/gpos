CC=gcc
LD=ld
AS=as
ARCHINCLUDE=$(PWD)/arch/x86/asm/include
BOOTLOADER_INCLUDE=${PWD}/bootloader/x86/include

ASMKFLAGS	= -I$(PWD)/include -I$(ARCHINCLUDE) -I$(BOOTLOADER_INCLUDE)
CFLAGS = -m32 -fno-builtin -fno-stack-protector -I$(PWD)/arch/x86/include $(ASMKFLAGS)

objs-fs=buffer.o super.o inode.o namei.o

all: $(objs-fs)

clean:
	rm -rf $(objs-fs)

.PHONY: all clean
