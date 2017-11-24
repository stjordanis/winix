#include "fs.h"

int rw_chunk(struct inode *ino, int off, int len, int curr_fp_index, char *buf, int flag) {
    int j;
    struct blk_buf *buffer = get_block(ino->i_dev, curr_fp_index);
    char c;
    if (flag & READING) {
        for (j = off; j< off + len; j++) {
			if (buffer->block[j] == EOF)
				return EOF;
			*buf++ = buffer->block[j];
        }
    }
    else {
        for (j = off; j< off + len; j++) {
            buffer->block[j] = *buf++;
        }
		buffer->block[j] = EOF;
    }
	
	return OK;
}

int rw_file(struct filep *filp, char *buf, size_t count, int flag){
    // char *pbuf = get_physical_addr(buf);
    int ret, r;
    int open_slot, pos;
	struct bdev* dev = filp->filp_ino->i_dev;
    
    int b,off, len;
    struct blk_buf *buffer;
	block_t curr_fp_index, end_index, cur_blk;
    struct inode *ino = NULL;
    char c;
    int j;

    curr_fp_index = filp->filp_pos / BLOCK_SIZE;
    off = filp->filp_pos % BLOCK_SIZE;
	end_index = (filp->filp_pos + count + 1) / BLOCK_SIZE;
    ino = filp->filp_ino;

    for( ; curr_fp_index <= end_index; curr_fp_index++){
        len = BLOCK_SIZE - off > count ? count : BLOCK_SIZE - off;
		cur_blk = filp->filp_ino->i_zone[curr_fp_index];

		if (cur_blk == 0) {
			if (flag & WRITING) {
				if (curr_fp_index < NR_TZONES) {
					buffer = alloc_block(dev);
					cur_blk = buffer->b_blocknr;
					filp->filp_ino->i_zone[curr_fp_index] = cur_blk;
				}
				else {
					return EFBIG;
				}
			}
			else { //read is too large
				*buf = EOF;
				break;
			}
		}

        /* Read or write 'chunk' bytes. */
        r = rw_chunk(ino,  off, len, cur_blk, buf, flag);
        if (r != OK) 
            break;    /* EOF reached */

        count -= len;
		buf += len;
        filp->filp_pos += len;
        off = 0;
    }
    return OK;
}

int sys_read(struct proc *who,int fd, void *buf, int count){
    rw_file(who->fp_filp[fd], buf,count, READING);
}

int sys_write(struct proc *who,int fd, void *buf, int count){
    rw_file(who->fp_filp[fd], buf, count, WRITING);
}


