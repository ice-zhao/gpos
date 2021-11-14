#include <bss.h>
#include <string.h>

void init_bss_section(void) {
    memset(bss_start_ptr, 0, bss_end_ptr - bss_start_ptr);
}
