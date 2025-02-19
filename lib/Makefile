include $(BASE)/arch/$(ARCH)/asm/include/Makefile


obj-lib = lib$(libname).a

objs := errno.o open.o ctype.o dup.o write.o print.o

all: $(obj-lib)

errno.o: errno.c
	$(CC) $(CFLAGS) -c $< -o $@

open.o: open.c
	$(CC) $(CFLAGS) -c $< -o $@

ctype.o: ctype.c
	$(CC) $(CFLAGS) -c $< -o $@

dup.o: dup.c
	$(CC) $(CFLAGS) -c $< -o $@

write.o: write.c
	$(CC) $(CFLAGS) -c $< -o $@

print.o: print.c
	$(CC) $(CFLAGS) -c $< -o $@

$(obj-lib): $(objs)
	$(AR) --target=$(ar_target) crs $@ $^

clean:
	rm -rf $(objs) $(obj-lib)


.PHONY: all clean
