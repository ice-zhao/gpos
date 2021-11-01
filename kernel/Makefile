
include $(BASE)/arch/$(ARCH)/asm/include/Makefile

output := $(BASE)/$(DEST)
objs := schedule.o fork.o
CFLAGS += -I $(BASE)/arch/$(ARCH)/

all: $(objs)


schedule.o: schedule.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

fork.o: fork.c $(BASE)/include/kernel/*.h $(BASE)/arch/$(ARCH)/asm/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(objs)



.PHONY: all clean
