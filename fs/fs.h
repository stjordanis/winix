#ifndef _FS_FS_H_
#define _FS_FS_H_ 1

#define _CRT_NO_TIME_T 1

//#define NULL ((void*)0)
#include <stdio.h>

#include <kernel\kernel.h>
#include "dir.h"
#include "stat.h"
#include <winix/device.h>
#include <fs/const.h>
#include <fs/inode.h>
#include <fs/cache.h>
#include <fs/file.h>
#include <fs/dev.h>
#include <fs/path.h>
#include <fs/filp.h>
#include <fs/makefs.h>
#include <fs/super.h>

#include <fs/open.h>
#include <fs/read.h>
#include <fs/close.h>


#define ROOT_INO	(1)

/* open-only flags */
#define    O_RDONLY     0x0000        /* open for reading only */
#define    O_WRONLY     0x0001        /* open for writing only */
#define    O_RDWR       0x0002        /* open for reading and writing */
#define    O_ACCMODE    0x0003        /* mask for above modes */
#define    O_CREAT      0x0200        /* create if nonexistant */


int hexstr2int(char *a, int len);
char hexstr2char(char A);
void int2hexstr(char *buffer,int n, int bytenr);
int init_dir(struct blk_buf*);

#endif

