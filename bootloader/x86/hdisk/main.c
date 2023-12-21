#include <print.h>
#include <fs/fs.h>

static const long buffer_memory_end = 4*1024*1024;    //4M

void main() {
    buffer_init(buffer_memory_end);
    iprintk("Hi, starting from hard disk.\n");

    for(;;);
}
