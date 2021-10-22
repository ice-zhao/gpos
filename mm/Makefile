include $(BASE)/arch/$(ARCH)/asm/include/Makefile
CFLAGS += -I $(BASE)/include/


all: mm.o

heap.o: heap.c $(BASE)/include/mm/heap.h
	$(CC) $(CFLAGS) -o $@ -c $<

objs := heap.o

mm.o: $(objs)

clean:
	rm -rf $(objs) mm.o

.PHONY: all clean
