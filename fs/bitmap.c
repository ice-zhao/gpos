#include <string.h>
#include <print.h>
#include <kernel/kernel.h>
#include <fs/fs.h>

#define clear_bit(nr,addr) ({                   \
register int res ; \
__asm__ __volatile__("btrl %2,%3\n\tsetnb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})


void free_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;

	if (!inode)
		return;
	if (!inode->i_dev) {
		memset(inode,0,sizeof(*inode));
		return;
	}
	if (inode->i_count>1) {
		/* printk("trying to free inode with count=%d\n",inode->i_count); */
		iprintk("trying to free inode with count bigger one\n");
		panic("free_inode\n");
	}
	if (inode->i_nlinks)
		panic("trying to free inode with links\n");
	if (!(sb = get_super(inode->i_dev)))
		panic("trying to free inode on nonexistent device\n");
	if (inode->i_num < 1 || inode->i_num > sb->s_ninodes)
		panic("trying to free inode 0 or nonexistant inode\n");
    /* 1 block=2 sectors = 1024KB; 1024KB*8bit=8192(>>13) */
	if (!(bh=sb->s_imap[inode->i_num>>13]))
		panic("nonexistent imap in superblock\n");
	if (clear_bit(inode->i_num&8191,bh->b_data))
		iprintk("free_inode: bit already cleared.\n");
	bh->b_dirt = 1;
	memset(inode,0,sizeof(*inode));
}
