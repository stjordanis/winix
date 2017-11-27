#ifndef _FS_FS_H_
#define _FS_FS_H_ 1

#define _CRT_NO_TIME_T 1

//#define NULL ((void*)0)
#include <stdio.h>

#include <kernel\kernel.h>
#include "dir.h"
#include "stat.h"
#include <fcntl.h>
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


int hexstr2int(char *a, int len);
char hexstr2char(char A);
void int2hexstr(char *buffer,int n, int bytenr);
int init_dir(struct blk_buf*);

#endif

