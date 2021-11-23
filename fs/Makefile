include $(BASE)/arch/$(ARCH)/asm/include/Makefile


objs := buffer.o

all: $(objs)

buffer.o: buffer.c
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	rm -rf $(objs)


.PHONY: all clean
