#ifndef __MACHINE_H__
#define __MACHINE_H__

#define MACH_DATA_BASE      0x9000
#define MACH_DATA_PHY       (MACH_DATA_BASE * 0x10)

#define MEM_SIZE_OFFSET     0x200
#define MEM_SIZE_PHY        (MACH_DATA_PHY + MEM_SIZE_OFFSET)




#define __get_mem_size()    (*(unsigned long*)MEM_SIZE_PHY)


struct machine_data {
    unsigned long total_mem_size;   //in KB Unit;
};

extern struct machine_data mach_data;

void init_machine_data();
#endif
