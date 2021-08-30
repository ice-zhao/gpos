CC=gcc
LD=ld
AS=as
PWD=$(shell pwd)
KERNELLD=$(PWD)/kernel_x86.ld
OBJCOPY=objcopy
FDIMAGE=$(PWD)/../../utils/gpos.img
ARCHINCLUDE=$(PWD)/asm/include
floppy=$(PWD)/../../bootloader/x86/floppy

ASMKFLAGS	= -I include/ -I $(ARCHINCLUDE)

include asm/include/Makefile

all:  kernel.bin

asm/head.o: asm/head.S
	$(CC) $(ASMKFLAGS) -c $< -o $@

asm/kernel.elf: asm/head.o
	$(LD) -T$(KERNELLD) $(LDFLAGS) $< -o $@
	$(Q)nm $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > kernel.map


kernel.bin: asm/kernel.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $< $@


install: kernel.bin
	sudo mount $(FDIMAGE) $(floppy)
	sudo cp -rf $< $(floppy)
	sudo umount $(floppy)

clean:
	rm -rf asm/head.o asm/kernel.elf kernel.bin
