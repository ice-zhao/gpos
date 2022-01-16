#LDFLAGS=-Map=system.map
ASMKFLAGS = -I include/ -m32
LDFLAGS = -m elf_i386 -nolibc -nostdlib -L$(BASE)/lib/
CFLAGS = -m32 -fno-builtin -fno-stack-protector
CFLAGS += -I include/ -I $(BASE)/include/ -I $(BASE)/arch/$(ARCH)/include/ -I $(BASE)/arch/$(ARCH)/
Q=@
ar_target=elf32-i386
ld_debug=--verbose

libname=library
