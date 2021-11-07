#include <machine.h>


struct machine_data mach_data;

void init_machine_data(void) {
    mach_data.total_mem_size = __get_mem_size();
}
