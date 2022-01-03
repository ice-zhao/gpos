include $(BASE)/arch/$(ARCH)/asm/include/Makefile


objs := buffer.o super.o file_table.o bitmap.o inode.o

all: $(objs)

buffer.o: buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

super.o: super.c
	$(CC) $(CFLAGS) -c $< -o $@

file_table.o: file_table.c
	$(CC) $(CFLAGS) -c $< -o $@

bitmap.o: bitmap.c
	$(CC) $(CFLAGS) -c $< -o $@

inode.o: inode.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(objs)


.PHONY: all clean
