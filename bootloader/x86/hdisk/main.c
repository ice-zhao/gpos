#include <print.h>
#include <fs/fs.h>
#include <hdisk.h>
#include <string.h>

static const long buffer_memory_end = 4*1024*1024;    //4M

extern struct super_block *super;

void load_bin(unsigned short ino) {
	struct d_inode *inode = get_inode(ino);
	char *kernel_addr = (char*)KERNEL_START_ADDRESS;
	int i = 0;
	int z = 0;
	unsigned short* data_zone = (unsigned short*)(0x400000);  /* 4M */
	unsigned short* data_zone2 = (unsigned short*)(0x400400);  /* 4M + 1K */

	if (inode->i_size == 0) {
		iprintk("file size is zero!\n");
		return;
	}

	for (i = 0; i <=6; ++i) {
		if (inode->i_zone[i] == 0)
			break;

		read_blocks(inode->i_zone[i], 1, kernel_addr);
		kernel_addr += BLOCK_SIZE;
	}

	i=7;
    if (inode->i_zone[i] != 0) {
		read_blocks(inode->i_zone[i], 1, (char*)data_zone);
		for (z = 0; z < ZONES_PER_BLOCK; z++) {
			data_zone += z;
			if (*data_zone == 0)
				break;

			read_blocks(*data_zone, 1, kernel_addr);
			kernel_addr += BLOCK_SIZE;
		}
	}

	i=8;
    if (inode->i_zone[i] != 0) {
		read_blocks(inode->i_zone[i], 1, (char*)data_zone);
		for (i = 0; i < ZONES_PER_BLOCK; i++) {
			data_zone += i;
			if (*data_zone == 0)
				break;
			read_blocks(*data_zone, 1, (char*)data_zone2);
			for (z = 0; z < ZONES_PER_BLOCK; z++) {
				data_zone2 += z;
				if (*data_zone2 == 0)
					break;
				read_blocks(*data_zone2, 1, kernel_addr);
				kernel_addr += BLOCK_SIZE;
			}
		}
	}
}


void main() {
    buffer_init(buffer_memory_end);
	super = get_super(0);

	struct dir_entry *entry = path_lookup("/kernel.bin");

	if (entry) {
		load_bin(entry->inode);
	}

    for(;;);
}
