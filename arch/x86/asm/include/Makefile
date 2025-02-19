#LDFLAGS=-Map=system.map
ASMKFLAGS = -I include/ -m32
LDFLAGS = -m elf_i386 -z nodefaultlib -L$(BASE)/lib -L$(BASE)/drivers/char/tty
CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib
CFLAGS += -I include/ -I $(BASE)/include/ -I $(BASE)/arch/$(ARCH)/include/ -I $(BASE)/arch/$(ARCH)/
CPP = cpp -nostdinc
Q=@
ar_target=elf32-i386
ld_debug=--verbose

libname=library
chardrivers=chardrivers
