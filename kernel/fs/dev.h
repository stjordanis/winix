#ifndef _DEV_DEV_H_
#define _DEV_DEV_H_

#define DEV_READ    1
#define DEV_WRITE   2

#define align_sector(x) (((((x)-1)>>9)<<9))

int dev_io(char *buf, block_t blocknr,mode_t mode);

#endif