#include <fs/fs.h>
#include <hdisk.h>

extern struct super_block *super;

/*
 * ino: inode number
 root inode is 1
 */
struct d_inode* get_inode(unsigned int ino) {
	int block_idx = ino / INODES_PER_BLOCK;
	ino = ino % INODES_PER_BLOCK;

	int block = 2 +	super->s_imap_blocks + super->s_zmap_blocks + block_idx;
	struct d_inode* inodes = (struct d_inode*)(0x200000); /* 2M */
	read_blocks(block, 1, (char*)(inodes));
	return inodes + (ino - 1);
}
