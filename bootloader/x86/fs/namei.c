#include <kernel/mm_types.h>
#include "print.h"
#include "sys/stat.h"
#include <fs/fs.h>
#include <string.h>
#include <hdisk.h>

/* for now, always start from root as the search point
   the '/' is a virtual directory, no correspond dir_entry.
 */
struct dir_entry* path_lookup(char* path) {
	if (path[0] != '/')
		return NULL;

	char *part = NULL;
	while (*path == '/')
		path++;
	part = path;

	struct d_inode *inode = NULL;
	int len =0;
	int ino = 1;	/* root inode */
	int is_found = 0;
	struct dir_entry *entry = (struct dir_entry*)(0x300000);	/* 3M as buffer */
	while(*path != '\0') {
		len = 0;
		while (*path != '/' && *path != '\0') {
			path++;
			len++;
		}

		inode = get_inode(ino);
		int i = 0;
		int item = 0;
		is_found = 0;
		entry = (struct dir_entry*)(0x300000);
		/* only consider first 6 blocks for simple now enough */
		for (i = 0; i <= 6; ++i) {
			if (inode->i_zone[i] == 0)
				break;

			read_blocks(inode->i_zone[i], 1, (char*)entry);
			for (item = 0; item < DIR_ENTRIES_PER_BLOCK; ++item, entry++) {
				if (entry->inode == 0)
					break;

				if (strncmp(part, entry->name, len) == 0) {
					is_found = 1;
					break;
				}
			}

			if (is_found) {
				ino = entry->inode;
				break;
			}
		}

		if (!is_found) {
			entry = NULL;
			break;
		}

		while (*path == '/')
			path++;
		part = path;
	}

	return entry;
}
