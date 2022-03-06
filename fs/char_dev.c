#include <sys/types.h>
#include <kernel/tty.h>
#include <fs/fs.h>
#include <errno.h>

typedef int (*crw_ptr)(int rw,unsigned minor,char * buf,int count,off_t * pos);

static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos)
{
	/* return ((rw==READ)?tty_read(minor,buf,count): */
	/* 	tty_write(minor,buf,count)); */
    return tty_write(minor,buf,count);
}

static crw_ptr crw_table[]={
	NULL,		/* nodev */
	NULL,	/* /dev/mem etc */
	NULL,		/* /dev/fd */
	NULL,		/* /dev/hd */
	rw_ttyx,	/* /dev/ttyx */
	NULL,		/* /dev/tty */
	NULL,		/* /dev/lp */
	NULL};		/* unnamed pipes */


#define NRDEVS ((sizeof (crw_table))/(sizeof (crw_ptr)))


int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
	crw_ptr call_addr;

	if (MAJOR(dev)>=NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[MAJOR(dev)]))
		return -ENODEV;
	return call_addr(rw,MINOR(dev),buf,count,pos);
}
