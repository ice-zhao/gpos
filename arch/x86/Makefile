CC=gcc
LD=ld
AS=as
PWD=$(shell pwd)
KERNELLD=$(PWD)/kernel_x86.ld
OBJCOPY=objcopy
FDIMAGE=$(BASE)/utils/gpos.img
ARCHINCLUDE=$(PWD)/asm/include
floppy=$(BASE)/bootloader/x86/floppy

include asm/include/Makefile

ASMKFLAGS += -I $(ARCHINCLUDE)

output := $(BASE)
output-kernel := $(output)/kernel
output-mm := $(output)/mm
output-lib := $(output)/lib
output-hd-drivers := $(output)/drivers/block/hardisk
output-fs := $(output)/fs

CFLAGS += -I $(BASE)/arch/$(ARCH)/ -I $(BASE)/include/

all:  kernel.bin

objs-kernel := $(output-kernel)/schedule.o $(output-kernel)/fork.o $(output-kernel)/trap.o
objs-mm := $(output-mm)/heap.o $(output-mm)/memory.o $(output-mm)/page.o
objs-lib := $(output-lib)/errno.o
objs := asm/iprintk.o bss.o main.o asm/system_call.o machine.o
objs-drivers := $(output-hd-drivers)/hd.o $(output-hd-drivers)/ll_rw_blk.o
objs-fs := $(output-fs)/buffer.o
objs-all := $(objs-kernel) $(objs-mm) $(objs-lib) $(objs-drivers) $(objs-fs)

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
	$(LD) -T$(KERNELLD) $(LDFLAGS) $^ -o $@
	$(Q)nm $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > kernel.map


kernel.bin: asm/kernel.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $< $@


install: kernel.bin
	sudo mount $(FDIMAGE) $(floppy)
	sudo cp -rf $< $(floppy)
	sudo umount $(floppy)

clean:
	rm -rf asm/kernel.elf kernel.bin asm/head.o $(objs)
