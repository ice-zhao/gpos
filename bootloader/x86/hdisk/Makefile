CC=gcc
LD=ld
AS=as
BOOTLDFILE=$(PWD)/bootloader/x86/boot_x86.ld
OBJCOPY=objcopy
HDIMAGE=$(PWD)/utils/hd.img
ARCHINCLUDE=$(PWD)/arch/x86/asm/include
BOOTLOADER_INCLUDE=${PWD}/bootloader/x86/include

ASMKFLAGS	= -I$(PWD)/include/ -I$(ARCHINCLUDE) -I$(BOOTLOADER_INCLUDE)
LDFLAGS = -m elf_i386 -nolibc -nostdlib
CFLAGS = -m32 -fno-builtin -fno-stack-protector -I ${PWD}/arch/x86/include

objs-hdisk=hboot.o hdloader.o main.o hd.o

all: $(objs-hdisk) hboot.bin

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

main.o: main.c
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<

hd.o: hd.c
	$(CC) $(ASMKFLAGS) $(CFLAGS) -c $<

clean:
	rm -rf hboot.bin hboot.elf hboot.o hdloader.bin hdloader.o hdloader.elf
	rm -rf hd.o main.o

.PHONY: clean all
