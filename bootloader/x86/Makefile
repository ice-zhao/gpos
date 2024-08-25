CC=gcc
LD=ld
AS=as
BOOTLDFILE=$(PWD)/bootloader/x86/boot_x86.ld
LOADERLDFILE=$(PWD)/bootloader/x86/loader_x86.ld
OBJCOPY=objcopy
FDIMAGE=$(PWD)/utils/gpos.img
floppy=$(BASE)/bootloader/x86/floppy
ARCHINCLUDE=$(PWD)/arch/x86/asm/include

HD_LOADERLDFILE=$(PWD)/bootloader/x86/hdisk/hdloader_x86.ld

include include/Makefile

ASMKFLAGS	= -I include/ -I $(ARCHINCLUDE)
CFLAGS = -m32 -fno-builtin -fno-stack-protector -I ${PWD}/arch/x86/include

bin-bootloader=boot.bin loader.bin
hdisk-path=./hdisk
objs-hdisk= $(hdisk-path)/hdloader.o $(hdisk-path)/hboot.o \
			$(hdisk-path)/main.o $(hdisk-path)/hd.o

objs-core=./core/iprintk.o
objs-fs=./fs/buffer.o ./fs/super.o ./fs/inode.o ./fs/namei.o

all:
	$(Q)make all -C ./fs
	$(Q)make all -C ./core
	$(Q)make all -C ./hdisk
	$(Q)make $(hdisk-path)/hdloader.bin
	$(Q)make $(bin-bootloader)

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
	sudo mount $(FDIMAGE) $(floppy)
	sudo cp -rf loader.bin $(floppy)
	sudo umount $(floppy)

$(hdisk-path)/hdloader.elf: $(objs-hdisk) $(objs-core) $(objs-fs)
	$(LD) $(LDFLAGS) -T$(HD_LOADERLDFILE) $^ -o $@

$(hdisk-path)/hdloader.bin: $(hdisk-path)/hdloader.elf
	$(OBJCOPY) -R .pdr -R .comment -R .note -S -O binary $^ $@

clean:
	rm -rf boot.o boot.elf boot.bin loader.o loader.elf loader.bin
	$(Q)make clean -C ./fs
	$(Q)make clean -C ./core
	$(Q)make clean -C $(hdisk-path)
