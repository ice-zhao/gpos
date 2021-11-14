include $(BASE)/arch/$(ARCH)/asm/include/Makefile
CFLAGS += -I $(BASE)/include/ -I $(BASE)/arch/$(ARCH)/

all: mm.o

heap.o: heap.c $(BASE)/include/mm/heap.h
	$(CC) $(CFLAGS) -o $@ -c $<


memory.o: memory.c $(BASE)/include/mm/*.h $(BASE)/arch/$(ARCH)/include/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

page.o: page.s
	$(CC) $(ASMKFLAGS) -o $@ -c $<

objs := heap.o memory.o page.o

mm.o: $(objs)

clean:
	rm -rf $(objs)

.PHONY: all clean
