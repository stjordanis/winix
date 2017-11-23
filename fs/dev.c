#include <fs/dev.h>
#include <fs/makefs.h>
#include "fs.h"

char disk[TOTAL_SIZE];

int dev_io(struct bdev* dev, char *buf, block_t blocknr,mode_t mode){
    int i = blocknr * BLOCK_SIZE * 4;
    int *disk_buf = (int *)(disk + i);
//    i = align_sector(i);
    char *limit = buf + BLOCK_SIZE;
    if(mode == DEV_READ){
		for (i = 0; i < BLOCK_SIZE; i++)
		{
			*buf++ = *disk_buf++;
		}
    }else if (mode == DEV_WRITE){
		for (i = 0; i < BLOCK_SIZE; i++)
		{
			*disk_buf++ = *buf++;
		}
    }
	return OK;
}


