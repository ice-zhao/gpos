include $(BASE)/arch/$(ARCH)/asm/include/Makefile
APPS_LDS = $(BASE)/apps/apps.ld

# LDFLAGS = -z nodefaultlib -L$(BASE)/lib

all: hello

hello: hello.o $(BASE)/kernel/vsprintf.o
	$(LD) -Ttext=0 -T$(APPS_LDS) $(LDFLAGS) $^ -l$(libname) -static -o $@


hello.o: hello.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf hello hello.o

.PHONY: clean all
