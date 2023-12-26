#ifndef HDISK_H
#define HDISK_H
#include <fs/fs.h>
/*
 * read blocks from hard disk
 Parms:
	block_idx: block index of one partition to read, starting from 0
	for minix, index 0 stands for bootstrap block, index 1 is super block.
	counts: the total of blocks, max value: 127
	buffer: disk content will be saved.
 */
void read_blocks(unsigned int block_idx, unsigned int counts, char* buffer);

struct d_inode* get_inode(unsigned int ino);

void DispInt(int);

struct dir_entry* path_lookup(char* path);

#define KERNEL_START_ADDRESS (0x800000) /* 8M */
#define ZONES_PER_BLOCK	(BLOCK_SIZE/sizeof(unsigned short))
#endif
