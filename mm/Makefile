include $(BASE)/arch/$(ARCH)/asm/include/Makefile
CFLAGS += -I $(BASE)/include/ -I $(BASE)/arch/$(ARCH)/


all: mm.o

heap.o: heap.c $(BASE)/include/mm/heap.h
	$(CC) $(CFLAGS) -o $@ -c $<


memory.o: memory.c $(BASE)/include/mm/heap.h $(BASE)/arch/$(ARCH)/include/machine.h
	$(CC) $(CFLAGS) -o $@ -c $<

objs := heap.o memory.o

mm.o: $(objs)

clean:
	rm -rf $(objs)

.PHONY: all clean
