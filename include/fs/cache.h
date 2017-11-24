#ifndef _FS_LRU_H_
#define _FS_LRU_H_ 1

#include <winix/device.h>

struct blk_buf
{
    int block[BLOCK_SIZE];
    struct blk_buf *prev, *next;
    block_t b_blocknr; // block number for this buffer
    struct bdev* b_dev;            /* major | minor device where block resides */
    int b_dirt; // clean or dirty
    int b_count; // number of users on this buffer
};


#define LRU_LEN         16

#define FRONT   1
#define REAR    0

/* When a block is released, the type of usage is passed to put_block(). */
#define WRITE_IMMED   1 /* block should be written to disk now */
#define ONE_SHOT      2 /* set if block not likely to be needed soon */


struct blk_buf *get_imap(struct bdev*);
int put_imap(struct bdev*);
struct blk_buf *get_bmap(struct bdev*);
int put_bmap(struct bdev*);
int put_block(struct bdev*, struct blk_buf *buffer, mode_t mode);
struct blk_buf *get_block(struct bdev*,block_t blocknr);
void enqueue_buf(struct blk_buf *tbuf);
struct blk_buf* dequeue_buf();
int free_block(struct bdev* dev, struct blk_buf* buf);
struct blk_buf* alloc_block(struct bdev* dev);
void init_buf();

#endif
