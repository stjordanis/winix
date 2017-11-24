#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <fs/makefs.h>
#include <winix/bitmap.h>


void init_super(size_t totalsiz, struct bdev* dev) {

	size_t totalsiz_inbytes, totalblocks, blockmap_blksiz,
		remainingblocks, inodetable_blksiz, inodemap_blksiz,
		 currblk;
	struct super_block* sb = &dev->sb;

	ASSERT(totalsiz > 262144); //at least 256 KB

	sb->s_magic = WINIX_MAGIC;
	memcpy(&dev->sb.s_name, "WFS", 4);
	sb->s_block_size = PAGE_LEN;
	sb->s_inode_size = DISK_INODE_SIZ;
	sb->s_inode_per_block = PAGE_LEN / DISK_INODE_SIZ;	// PAGE_LEN / 128
	sb->s_super_idx = 1;
	sb->s_super_nr = 1;
	sb->s_blockmap_idx = 2;

	dev->sb.s_total_size = totalsiz;
	totalsiz_inbytes = totalsiz;
	totalsiz /= 4;	//everything is addressed in words

	currblk = sb->s_blockmap_idx;
	
	totalblocks = totalsiz / PAGE_LEN;
	blockmap_blksiz = totalblocks / PAGE_LEN / 32;
	remainingblocks = totalblocks;
	inodetable_blksiz = totalsiz_inbytes / 16384  * sb->s_inode_size / PAGE_LEN;
	inodemap_blksiz = inodetable_blksiz * sb->s_inode_per_block / PAGE_LEN / 32;

	one_if_zero(&inodemap_blksiz);
	one_if_zero(&inodetable_blksiz);
	one_if_zero(&blockmap_blksiz);

	// block map
	sb->s_blockmap_nr = blockmap_blksiz;
	currblk += blockmap_blksiz;

	//inode map
	sb->s_inodemap_idx = currblk;
	sb->s_inodemap_nr = inodemap_blksiz;
	currblk += inodemap_blksiz;

	//inode table
	sb->s_inodetable_idx = currblk;
	sb->s_inodetable_nr = inodetable_blksiz;
	currblk += inodetable_blksiz;
	
	remainingblocks = totalblocks - currblk;
	sb->s_block_inuse = currblk - 1;
	//s_inode_inuse not initialised

	sb->s_free_blocks = totalblocks - currblk;

	sb->s_nblock = currblk;
	
	//s_ninode, s_free_inodes not initialised
}
/*	
	Blocks are arranged as below
	0 - 1		boot sector
	1 - 2		super block sector
	2 - 3		block bitmap
	3 - 4		inode bitmap
	4 - 132		inode table
	132 - 4096	free blocks
*/

int makefs(struct bdev* dev)
{
    char *pdisk = disk;
	struct blk_buf* buf, *root_blk = NULL;
	struct super_block* sb = &dev->sb;
	struct inode* ino;
	int i;
	block_t curr;
	int offset;
	block_t blockoff, bitmapoff;
	
	init_super(TOTAL_SIZE,dev);
	kprintf_blkbuf("initla");

	// block map
	buf = get_block(dev, sb->s_blockmap_idx);
	bitmap_set_nbits(buf->block, PAGE_LEN, 0, sb->s_block_inuse);
	put_block(dev, buf, WRITE_IMMED);

	offset = ROOT_INO * sb->s_inode_size;
	blockoff = offset / PAGE_LEN;
	bitmapoff = blockoff / 32;
	offset /= PAGE_LEN;

	// inode map
	buf = get_block(dev, sb->s_inodemap_idx);
	bitmap_set_bit(buf->block, PAGE_LEN, ROOT_INO);
	put_block(dev, buf, WRITE_IMMED);

	buf = get_block(dev, sb->s_blockmap_idx);

	// root inode struct in inode table
	buf = get_block(dev, sb->s_inodetable_idx + blockoff);
	ino = read_inode(dev, ROOT_INO);

	root_blk = alloc_block(dev);
	ino->i_zone[0] = root_blk->b_blocknr;
	ino->i_mode = 0x00000fff ^ 0x22 | S_IFDIR;
	ino->i_nlinks = 1;
	ino->i_dev = dev;
	ino->i_count = 1;
	ino->i_num = ROOT_INO;
	ino->i_ndblock = buf->b_blocknr;
	put_block(dev, buf, WRITE_IMMED);	//write inode struct 
	
	//the block that is newly allocated
	if (root_blk) {
		init_dir(root_blk);
		ino->i_size = 2 * sizeof(struct direct);
	}


	
	curr = (sb->s_super_idx + sb->s_super_nr) * PAGE_LEN;
    printf("\nsuper block 0 - 0x%08x\n", curr);
    printf("block map 0x%08x - 0x%08x\n",curr, curr+ sb->s_blockmap_nr * PAGE_LEN );
    curr += sb->s_blockmap_nr * PAGE_LEN;
    printf("inode map 0%08x - 0x%08x\n",curr, curr+ sb->s_inodemap_nr * PAGE_LEN);
    curr += sb->s_inodemap_nr * PAGE_LEN;
    printf("inode table 0x%08x - 0x%08x\n",curr, curr+ sb->s_inodetable_nr * PAGE_LEN);
    curr += sb->s_inodetable_nr * PAGE_LEN;
    printf("data block 0x%08x - 0x%lx\n Number of free blocks %ld\n",curr, curr+sb->s_free_blocks * PAGE_LEN, sb->s_free_blocks);
    
    return OK;
}