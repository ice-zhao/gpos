#include "kernel/kernel.h"
#include <print.h>
#include <errno.h>
#include <fs/fs.h>
#include <sys/stat.h>
#include <kernel/schedule.h>

extern int rw_char(int rw,int dev, char * buf, int count, off_t * pos);

int sys_write(unsigned int fd,char * buf,int count)
{
	struct file * file;
	struct m_inode * inode;

	if (fd>=NR_OPEN || count <0 || !(file=current->filp[fd]))
		return -EINVAL;
	if (!count)
		return 0;
	inode=file->f_inode;
	/* if (inode->i_pipe) */
	/* 	return (file->f_mode&2)?write_pipe(inode,buf,count):-EIO; */
	if (S_ISCHR(inode->i_mode)) {
		return rw_char(WRITE,inode->i_zone[0],buf,count,&file->f_pos);
	}
	/* if (S_ISBLK(inode->i_mode)) */
	/* 	return block_write(inode->i_zone[0],&file->f_pos,buf,count); */
	/* if (S_ISREG(inode->i_mode)) */
	/* 	return file_write(inode,file,buf,count); */
	printk("(Write)inode->i_mode=%06o\n\r",inode->i_mode);
	return -EINVAL;
}
