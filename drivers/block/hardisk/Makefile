include $(BASE)/arch/$(ARCH)/asm/include/Makefile

CFLAGS += -I $(BASE)/arch/$(ARCH)/ -I $(BASE)/drivers/ -I $(BASE)/include/
objs := hd.o ll_rw_blk.o

all: $(objs)


hd.o: hd.c
	$(CC) $(CFLAGS)	-o $@ -c $<

ll_rw_blk.o: ll_rw_blk.c
	$(CC) $(CFLAGS)	-o $@ -c $<


clean:
	rm -rf $(objs)


.PHONY: all clean
