#include "fs.h"

int rw_chunk(struct inode *ino, int off, int len, int curr_fp_index, char *buf, int flag) {
    int j;
    struct blk_buf *buffer = get_block(ino->i_dev, curr_fp_index);
    char c;
    if (flag & READING) {
        for (j = off; j< off + len; j++) {
            *buf++ = buffer->block[j];
        }
    }
    else {
        for (j = off; j< off + len; j++) {
            buffer->block[j] = *buf++;
        }
    }
}

int rw_file(struct filep *filp, char *buf, size_t count, int flag){
    // char *pbuf = get_physical_addr(buf);
    int ret, r;
    int open_slot, pos;
    
    int b,off, len;
    struct blk_buf *buffer;
    block_t curr_fp_index;
    struct inode *ino = NULL;
    char c;
    int j;

    curr_fp_index = filp->filp_pos / BLOCK_SIZE;
    off = filp->filp_pos % BLOCK_SIZE;
    curr_fp_index = (filp->filp_pos + count - 1) / BLOCK_SIZE;
    ino = filp->filp_ino;

    for( ; curr_fp_index <= curr_fp_index; curr_fp_index++){
        len = BLOCK_SIZE - off > count ? count : BLOCK_SIZE - off;

        /* Read or write 'chunk' bytes. */
        r = rw_chunk(ino,  off, len, curr_fp_index, buf, flag);
        if (r != OK) 
            break;    /* EOF reached */

        count -= len;
        filp->filp_pos = off + len;

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


