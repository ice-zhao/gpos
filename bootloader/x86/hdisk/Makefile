CC=gcc
LD=ld
AS=as
#PWD=$(shell pwd)
BOOTLDFILE=$(PWD)/bootloader/x86/boot_x86.ld
LOADERLDFILE=$(PWD)/bootloader/x86/hdisk/hdloader_x86.ld
OBJCOPY=objcopy
HDIMAGE=$(PWD)/utils/hd.img
ARCHINCLUDE=$(PWD)/arch/x86/asm/include
BOOTLOADER_INCLUDE=${PWD}/bootloader/x86/include

ASMKFLAGS	= -I include/ -I $(ARCHINCLUDE) -I $(BOOTLOADER_INCLUDE)
LDFLAGS = -m elf_i386 -nolibc -nostdlib
CFLAGS = -m32 -fno-builtin -fno-stack-protector


all: hboot.bin hdloader.bin

hboot.o: hboot.S
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<

hboot.elf: hboot.o
	$(LD) $(LDFLAGS) -T$(BOOTLDFILE) $< -o $@

hboot.bin: hboot.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $^ $@


#install boot sector, 1th sector
install: hboot.bin hdloader.bin
	dd if=hboot.bin of=$(HDIMAGE) bs=512 count=1 conv=notrunc
	dd if=hdloader.bin of=$(HDIMAGE) seek=1 bs=512 conv=notrunc

hdloader.o: hdloader.S
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<

hdloader.elf: hdloader.o
	$(LD) $(LDFLAGS) -T$(LOADERLDFILE) $< -o $@

hdloader.bin: hdloader.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $^ $@

clean:
	rm -rf hboot.bin hboot.elf hboot.o hdloader.bin hdloader.o hdloader.elf

.PHONY: clean all
