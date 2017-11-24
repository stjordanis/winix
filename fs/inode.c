#include "fs.h"
#include <fs/inode.h>
#include <winix/bitmap.h>

struct inode inodetable[NR_INODES];


struct inode* read_inode(struct bdev* dev, ino_t num){
	struct super_block* sb = &dev->sb;
    block_t blocknr = sb->s_inodetable_idx + num / sb->s_inode_per_block;
    int offset = (num) % sb->s_inode_per_block * sb->s_inode_size;
    char *start;
    int *ino_buf;
    struct blk_buf *buffer;
    struct inode *ino = NIL_INODE;

    for(ino = &inodetable[0]; ino < &inodetable[NR_INODES]; ino++){
        if(ino->i_num == 0)
            break;
    }
    
    if(ino == NIL_INODE)
        return NIL_INODE;

    // printf("free inode found\n");
    
    buffer = get_block(dev, blocknr);
    ino_buf = (int *)ino;
	memcpy(ino_buf, buffer + offset, DISK_INODE_SIZ);
	return ino;
}

struct inode* get_inode(struct bdev* dev, ino_t num){
    struct inode* rep;
    int i=0;
    struct blk_buf *imap;
    char val;
    for(rep = &inodetable[0]; rep < &inodetable[NR_INODES]; rep++ ){
        if(rep->i_num == num)
            return rep;
    }
    
	imap = get_block(dev, dev->sb.s_inodemap_idx);
	if (is_bit_on(imap->block, PAGE_LEN, num)) {
		rep = read_inode(dev, num);
		rep->i_num = num;
		return rep;
	}
    
    return NULL;
}






int put_inode(struct bdev* dev, struct inode *inode){
	struct super_block* sb = &dev->sb;
    int inum = inode->i_num;
    block_t blocknr = inode->i_ndblock;
    int inode_block_offset = (inum) % sb->s_inode_per_block * sb->s_inode_size;
    char *bak;
	int i;

    struct blk_buf *buffer = get_block(dev, blocknr);
    char *buf = &buffer->block[inode_block_offset];
    // offset in terms of byte index, each sector in inode table contains four inode
    
	memcpy(buf, inode, DISK_INODE_SIZ);
    
    return put_block(dev, buffer,WRITE_IMMED);
}


struct inode* alloc_inode(struct bdev* dev ){
	struct super_block* sb = &dev->sb;
    int inum = sb->s_ninode; // next free inode
    block_t iblock = sb->s_nblock;
    struct blk_buf *imap, *bmap, *blkbuf;
    struct inode *inode;
    int i;

    // TODO: if inum is negative, (no inode available), return NULL
    for(inode = &inodetable[0]; inode < &inodetable[NR_INODES]; inode++){
        if(inode->i_num == 0){ // if inode is free
            break;
        }
    }

    imap = get_block(dev, sb->s_inodemap_idx);
	inum = bitmap_search_from(imap->block, PAGE_LEN, 1, 1);
	if (inum == ERR)
		return NULL;

    // allocate new inode and block

	bitmap_set_bit(imap->block, PAGE_LEN, inum);
    //imap->block[inum/32] |= (0x80000000) >> (inum%32);

    if(put_block(dev,imap, WRITE_IMMED) == 0){
		if (blkbuf = alloc_block(dev)) {
			inode->i_dev = dev;
			
			inode->i_zone[0] = iblock;
			inode->i_num = inum;
			inode->i_ndblock = sb->s_inodetable_idx + (inum / sb->s_inode_per_block);
			return inode;
		}        
    }
    return NIL_INODE;
}

struct inode* new_inode_in_path(struct bdev* dev, char* path) {
	char string[DIRSIZ];
	struct direct* dir;
	struct blk_buf* buf;
	struct inode* ldip = last_dir(path, string);
	struct inode* ino;

	if (ino = advance(ldip, string)) {
		return ino;
	}

	ino = alloc_inode(dev);
	buf = get_block(dev, ldip->i_zone[0]);
	
	for (dir = (struct direct*)buf->block;
			dir < &buf->block[PAGE_LEN]; dir++)
	{
		if (dir->d_ino == 0) {
			dir->d_ino = ino->i_num;
			memcpy(dir->d_name, string, strlen(string) + 1);
			break;
		}
	}
	return ino;
}


void free_inode(struct inode *inode){
	struct bdev* dev = inode->i_dev;
	struct blk_buf* buf = get_block(dev, dev->sb.s_inodemap_idx);

	bitmap_clear_bit(buf->block, PAGE_LEN, inode->i_num);
	put_block(dev, buf, WRITE_IMMED);
	inode->i_num = 0;

	return OK;
}

void init_inode(){
    struct inode* rep;
    for(rep = &inodetable[0]; rep < &inodetable[NR_INODES]; rep++ ){
        rep->i_num = 0;
    }
}