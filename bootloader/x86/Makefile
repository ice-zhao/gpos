CC=gcc
LD=ld
AS=as
PWD=$(shell pwd)
BOOTLDFILE=$(PWD)/boot_x86.ld
LOADERLDFILE=$(PWD)/loader_x86.ld
OBJCOPY=objcopy
FDIMAGE=$(PWD)/../../utils/gpos.img
ARCHINCLUDE=$(PWD)/../../arch/x86/asm/include

ASMKFLAGS	= -I include/ -I $(ARCHINCLUDE)



all:  boot.bin loader.bin

#compile boot.S and write to boot sector of Floppy
boot.o: boot.S include/load.inc include/fat12hdr.inc
	$(CC) $(ASMKFLAGS) -c $<

boot.elf: boot.o
	$(LD) -o boot.elf -T$(BOOTLDFILE) boot.o

boot.bin: boot.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary boot.elf boot.bin

boot.img: boot.bin
	dd if=boot.bin of=$(FDIMAGE) bs=512 count=1 conv=notrunc

#compile loader.S
loader.o: loader.S $(ARCHINCLUDE)/pm.inc include/fat12hdr.inc include/load.inc
	$(CC) $(ASMKFLAGS) -o loader.o -c loader.S

loader.elf: loader.o
	$(LD) -o loader.elf -T$(LOADERLDFILE) loader.o
	$(Q)nm $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > loader.map


loader.bin: loader.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary loader.elf loader.bin

install: boot.img loader.bin
	sudo mount $(FDIMAGE) $(PWD)/floppy
	sudo cp -rf loader.bin $(PWD)/floppy
	sudo umount $(PWD)/floppy

clean:
	rm -rf boot.o boot.elf boot.bin loader.o loader.elf
