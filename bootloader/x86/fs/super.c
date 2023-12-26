#include <fs/fs.h>
#include <hdisk.h>

struct super_block *super = NULL;

struct super_block* get_super(int dev) {
	struct super_block* super = (struct super_block*)(0x100000);	//1M
	read_blocks(1, 1, (char*)(super));
	return super;
}
