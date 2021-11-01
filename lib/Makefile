include $(BASE)/arch/$(ARCH)/asm/include/Makefile


objs := errno.o

all: lib.o

errno.o: errno.c
	$(CC) $(CFLAGS) -c $< -o $@


lib.o: $(objs)

clean:
	rm -rf $(objs)


.PHONY: all clean
