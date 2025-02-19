
include $(BASE)/arch/$(ARCH)/asm/include/Makefile

output := $(BASE)/$(DEST)
objs := schedule.o fork.o trap.o panic.o vsprintf.o printk.o kernel.o
CFLAGS += -I $(BASE)/arch/$(ARCH)/


all: kernel.o

schedule.o: schedule.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

fork.o: fork.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

trap.o: trap.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

panic.o: panic.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

vsprintf.o: vsprintf.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

printk.o: printk.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

kernel.o: $(objs)
	$(AR) --target=$(ar_target) r $@ $^

clean:
	rm -rf $(objs)



.PHONY: all clean
