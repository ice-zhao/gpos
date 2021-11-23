#include <kernel/hdregs.h>
#include <asm/include/io.h>
#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

/* Max read/write errors/sector */
#define MAX_ERRORS	7
#define MAX_HD		2

static struct hd_struct {
	long start_sect;
	long nr_sects;
} hd[5*MAX_HD]={{0,0},};


/*
 *  This struct defines the HD's and their types.
 */
struct hd_i_struct {
	int head;   /* the number of header */
    int sect;   /* the number of sector per track */
    int cyl;    /* the number of cylinder */
    int wpcom;  /* 开始写前预补偿柱面号（乘4） */
    int lzone;  /* 磁头着陆(停止)柱面号 */
    int ctl;    /* 控制字节（驱动器步进选择） */
	};

struct hd_i_struct hd_info[] = { {0,0,0,0,0,0},{0,0,0,0,0,0} };
static int NR_HD = 0;

int sys_setup(void * BIOS)
{
	static int callable = 1;
	int i,drive;
	unsigned char cmos_disks;
	struct partition *p;
	struct buffer_head * bh;

	if (!callable)
		return -1;
	callable = 0;

	for (drive=0 ; drive<2 ; drive++) {
		hd_info[drive].cyl = *(unsigned short *) BIOS;
		hd_info[drive].head = *(unsigned char *) (2+BIOS);
		hd_info[drive].wpcom = *(unsigned short *) (5+BIOS);
		hd_info[drive].ctl = *(unsigned char *) (8+BIOS);
		hd_info[drive].lzone = *(unsigned short *) (12+BIOS);
		hd_info[drive].sect = *(unsigned char *) (14+BIOS);
		BIOS += 16;
	}

	if (hd_info[1].cyl)
		NR_HD=2;
	else
		NR_HD=1;

	for (i=0 ; i<NR_HD ; i++) {
		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects = hd_info[i].head*
				hd_info[i].cyl*hd_info[i].sect;
	}

	if ((cmos_disks = CMOS_READ(0x12)) & 0xf0)  //old style CHS-based disk
		if (cmos_disks & 0x0f)
			NR_HD = 2;
		else
			NR_HD = 1;
	else
		NR_HD = 0;

	for (i = NR_HD ; i < 2 ; i++) {
		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects = 0;
	}


}
