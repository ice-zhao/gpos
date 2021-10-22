#include <bss.h>

void init_bss_section(void) {
    char* p = NULL;

    for(p = _bss; p < _ebss; p++) {
        *p = 0;
    }
}
