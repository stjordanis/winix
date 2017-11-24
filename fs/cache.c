#include "fs.h"
#include <winix/bitmap.h>
static struct blk_buf buf_table[LRU_LEN];

static struct blk_buf *lru_cache[2];

// The lru is illustrated as below
// REAR -> next -> .... -> next -> FRONT
// With the most recently used cache at the front, and least recently used block at the rear

void rm_lru(struct blk_buf *buffer){
    if(buffer->prev)
        buffer->prev->next = buffer->next;

    if(buffer->next)
        buffer->next->prev = buffer->prev;
        
    if(lru_cache[REAR] == buffer)
        lru_cache[REAR] = buffer->next;
    
    if(lru_cache[FRONT] == buffer)
        lru_cache[FRONT] = buffer->prev;

	buffer->next = buffer->prev = NULL;
}

PRIVATE void buf_move_to_front(struct blk_buf *buffer){
    if(lru_cache[FRONT] == buffer)
        return;
    
    rm_lru(buffer);
    enqueue_buf(buffer);
}

PRIVATE void buf_move_to_rear(struct blk_buf *buffer) {
	struct blk_buf* rear = lru_cache[REAR];
	if (rear == buffer)
		return;

	rm_lru(buffer);
	buffer->next = rear;
	rear->prev = buffer;
	lru_cache[REAR] = buffer;
}

struct blk_buf* dequeue_buf() {
    struct blk_buf *rear = lru_cache[REAR];
    if (!rear)
        return NULL;

    rear->next->prev = NULL;
    lru_cache[REAR] = rear->next;

    return rear;
}

void enqueue_buf(struct blk_buf *tbuf) {
	struct blk_buf* front = lru_cache[FRONT];
    if (lru_cache[FRONT] == NULL) {
        tbuf = lru_cache[REAR] = lru_cache[FRONT] = &buf_table[0];
        tbuf->next = tbuf->prev = NULL;
        return;
    }

	front->next = tbuf;
    tbuf->prev = front;
    lru_cache[FRONT] = tbuf;

}



int put_block(struct bdev* dev, struct blk_buf *tbuf, mode_t mode) {
	buf_move_to_front(tbuf);
    if (mode == WRITE_IMMED) {
		tbuf->b_dirt = false;
		return dev_io(dev, tbuf->block, tbuf->b_blocknr, DEV_WRITE);
    }
	tbuf->b_dirt = true;
	return OK;
}

struct blk_buf *get_block(struct bdev* dev, block_t blocknr){
	struct blk_buf *tbuf = lru_cache[REAR];
	int ret;
	while (tbuf && tbuf->b_blocknr != blocknr) {
		tbuf = tbuf->next;
	}

	if (tbuf) {
		buf_move_to_front(tbuf);
		tbuf->b_count++;
		return tbuf;
	}
		

	//not in lru cache
    tbuf = dequeue_buf();
    if(tbuf && tbuf->b_dirt)
        put_block(dev, tbuf,WRITE_IMMED);

	memset(tbuf->block, 0, PAGE_LEN);
    tbuf->b_blocknr = blocknr;
    tbuf->next = tbuf->prev = NULL;
    tbuf->b_dirt = 0;
    tbuf->b_count = 0;

    if (dev_io(dev, tbuf->block, blocknr, DEV_READ)) {
        return NULL;
    }
        
    enqueue_buf(tbuf);
    return tbuf;
}

struct blk_buf* alloc_block(struct bdev* dev) {
	struct blk_buf* blkbuf;
	int i = dev->sb.s_blockmap_idx;
	int blklen = dev->sb.s_blockmap_nr;
	int idx;
	for (; i < i + blklen; i++)
	{
		blkbuf = get_block(dev, i);
		idx = bitmap_search(blkbuf->block, PAGE_LEN, 1);
		if (idx > 0) {
			bitmap_set_bit(blkbuf->block, PAGE_LEN, idx);
			dev->sb.s_free_blocks--;
			dev->sb.s_block_inuse++;
			put_block(dev, blkbuf, WRITE_IMMED);
			blkbuf = get_block(dev, idx);
			blkbuf->b_count = 1;
			blkbuf->b_dev = dev;
			return blkbuf;
		}
	}
	return NULL;
}

int free_block(struct bdev* dev, struct blk_buf* buf) {
	block_t blocknr = buf->b_blocknr;
	block_t blockoffset = blocknr / PAGE_LEN;
	struct blk_buf* blkmap;

	// write 0 to this block
	memset(buf->block, 0, PAGE_LEN);
	put_block(dev, buf, ONE_SHOT);

	// free the block in the block bitmap
	blkmap = get_block(dev, dev->sb.s_blockmap_idx + blockoffset);
	bitmap_clear_bit(blkmap->block, PAGE_LEN, blocknr % PAGE_LEN);
	put_block(dev, buf, WRITE_IMMED);
}

void kprintf_blkbuf(char *str) {
	int count = 0;
	struct blk_buf* buf = lru_cache[REAR];
	printf("%s\n", str);
	while (buf) {
		printf("block %d\n", buf->b_blocknr);
		buf = buf->next;
		count++;
	}
	printf("%d\n", count);
}

void init_buf(){
    int i=0;
    struct blk_buf *tbuf = NULL, *prevbuf = NULL;
    char *val;
    for(tbuf = &buf_table[0];tbuf< &buf_table[LRU_LEN];tbuf++){
        if(prevbuf == NULL){
            lru_cache[FRONT] = lru_cache[REAR] = tbuf;
            tbuf->next = tbuf->prev = NULL;
        }else{
            prevbuf->next = tbuf;
            tbuf->prev = prevbuf;
            lru_cache[FRONT] = tbuf;
        }
        prevbuf = tbuf;
    }

    //imap.b_blocknr = sb->s_inodemap_idx;
    //imap.b_dirt = 0;
    //dev_io(imap.block,imap.b_blocknr,DEV_READ);
    //for(val = &imap.block[0]; val < &imap.block[BLOCK_SIZE]; val++){
    //    *val = hexstr2char(*val);
    //}
    //// printf("imap %d\n",imap.block[0]);

    //bmap.b_blocknr = sb->s_blockmap_idx;
    //bmap.b_dirt = 0;
    //dev_io(bmap.block,bmap.b_blocknr,DEV_READ);
    //for(val = &bmap.block[0]; val < &bmap.block[BLOCK_SIZE]; val++){
    //    *val = hexstr2char(*val);
    //}
}


