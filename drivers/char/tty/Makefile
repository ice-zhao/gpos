include $(BASE)/arch/$(ARCH)/asm/include/Makefile

CFLAGS += -I $(BASE)/arch/$(ARCH)/ -I $(BASE)/drivers/ -I $(BASE)/include/
objs := tty_io.o console.o kb.o
obj-lib = lib$(chardrivers).a

all: $(obj-lib)


tty_io.o: tty_io.c
	$(CC) $(CFLAGS)	-o $@ -c $<

console.o: console.c
	$(CC) $(CFLAGS)	-o $@ -c $<

kb.o: keyboard.s
	$(CC) $(CFLAGS) -o $@ -c $<

keyboard.s: kb.S
	$(CPP) $(CFLAGS) kb.S -o keyboard.s

$(obj-lib): $(objs)
	$(AR) --target=$(ar_target) crs $@ $^

clean:
	rm -rf $(objs) keyboard.s $(obj-lib)


.PHONY: all clean
