#ifndef __HEAD_H__
#define __HEAD_H__

extern char* _pg_dir;

#define pgdir_table_ptr   ((unsigned long*)&_pg_dir)

typedef struct desc_struct {
	unsigned long a,b;
} desc_table[256];


extern desc_table idt;

#endif
