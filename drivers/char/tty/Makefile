include $(BASE)/arch/$(ARCH)/asm/include/Makefile

CFLAGS += -I $(BASE)/arch/$(ARCH)/ -I $(BASE)/drivers/ -I $(BASE)/include/
objs := tty_io.o

all: $(objs)


tty_io.o: tty_io.c
	$(CC) $(CFLAGS)	-o $@ -c $<

# ll_rw_blk.o: ll_rw_blk.c
# 	$(CC) $(CFLAGS)	-o $@ -c $<


clean:
	rm -rf $(objs)


.PHONY: all clean
