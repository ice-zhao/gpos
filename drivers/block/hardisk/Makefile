include $(BASE)/arch/$(ARCH)/asm/include/Makefile

CFLAGS += -I $(BASE)/arch/$(ARCH)/
objs := hd.o

all: $(objs)


hd.o: hd.c
	$(CC) $(CFLAGS)	-o $@ -c $<



clean:
	rm -rf $(objs)


.PHONY: all clean
