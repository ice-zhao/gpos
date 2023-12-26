CC=gcc
LD=ld
AS=as
PWD=$(shell pwd)
KERNELLD=$(PWD)/kernel_x86.ld
OBJCOPY=objcopy
FDIMAGE=$(BASE)/utils/gpos.img
ARCHINCLUDE=$(PWD)/asm/include
floppy=$(BASE)/bootloader/x86/floppy
hardisk=$(BASE)/utils/hd.img
loop1=/dev/loop1
disk-folder=$(BASE)/utils/disk

include asm/include/Makefile

ASMKFLAGS += -I $(ARCHINCLUDE)

output := $(BASE)
output-kernel := $(output)/kernel
output-mm := $(output)/mm
output-lib := $(output)/lib
output-hd-drivers := $(output)/drivers/block/hardisk
output-fs := $(output)/fs

CFLAGS += -I $(BASE)/arch/$(ARCH)/ -I $(BASE)/include/


objs-kernel := $(output-kernel)/schedule.o $(output-kernel)/fork.o $(output-kernel)/trap.o $(output-kernel)/panic.o \
			$(output-kernel)/vsprintf.o $(output-kernel)/printk.o
objs-mm := $(output-mm)/heap.o $(output-mm)/memory.o $(output-mm)/page.o
objs := asm/iprintk.o bss.o main.o asm/system_call.o machine.o
objs-drivers := $(output-hd-drivers)/hd.o $(output-hd-drivers)/ll_rw_blk.o

objs-fs := $(output-fs)/buffer.o $(output-fs)/super.o $(output-fs)/file_table.o $(output-fs)/inode.o $(output-fs)/bitmap.o \
			$(output-fs)/namei.o $(output-fs)/truncate.o $(output-fs)/open.o $(output-fs)/fcntl.o $(output-fs)/read_write.o \
			$(output-fs)/char_dev.o
objs-all := $(objs-kernel) $(objs-mm) $(objs-drivers) $(objs-fs)

all:  kernel.bin

asm/system_call.o: asm/system_call.S $(BASE)/include/kernel/sys.h $(BASE)/include/kernel/schedule.h
	$(CC) $(ASMKFLAGS) -o $@ -c $<

bss.o: bss.c
	$(CC) $(CFLAGS) -o $@ -c $^

asm/iprintk.o: asm/iprintk.S
	$(CC) $(ASMKFLAGS) -o $@ -c $^

main.o: main.c $(BASE)/include/unistd.h
	$(CC) $(CFLAGS) -o $@ -c $<

asm/head.o: asm/head.S
	$(CC) $(ASMKFLAGS) -o $@ -c $^

machine.o: machine.c
	$(CC) $(CFLAGS) -o $@ -c $^

asm/kernel.elf: asm/head.o $(objs) $(objs-all)
	$(LD) -T$(KERNELLD) $(LDFLAGS) $^ -l$(chardrivers) -l$(libname) -o $@
	$(Q)nm $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > kernel.map


kernel.bin: asm/kernel.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $< $@


install: kernel.bin
	sudo mount $(FDIMAGE) $(floppy)
	sudo cp -rf $< $(floppy)
	sudo umount $(floppy)

#1048576B is reserved for bootloader on disk
#use the `parted` comand to check this value.
hd_install: kernel.bin
	sudo losetup $(loop1) $(hardisk) -o 1048576
	sudo mount $(loop1) $(disk-folder)
	sudo cp -rf $< $(disk-folder)/$<
	sudo umount $(disk-folder)
	sudo losetup -d $(loop1)

clean:
	$(Q)rm -rf asm/kernel.elf kernel.bin asm/head.o $(objs)


.PHONY: all clean
