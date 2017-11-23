#ifndef _FS_SUPER_H_
#define _FS_SUPER_H_ 1

struct super_block {
    unsigned int s_magic;
    char s_name[32];
	size_t s_total_size;	//disk size in bytes

	size_t s_block_size;		// block size in words
	size_t s_inode_size;		// Inode size in words
	size_t s_inode_per_block;	

	block_t s_super_idx;	// super block index
	size_t	s_super_nr;		// num of blocks used by super block
    
    block_t s_blockmap_idx; // block bitmap block index
	size_t s_blockmap_nr;	// num of blocks used by block bitmap

    block_t s_inodemap_idx; // inode number bitmap block index
	size_t s_inodemap_nr;	// num of blocks used by inode bitmap

    block_t s_inodetable_idx; // inode table bitmap block index
	size_t s_inodetable_nr;	// num of blocks used by inode table

	size_t s_block_inuse;
	size_t s_inode_inuse;

	size_t s_free_blocks;
	size_t s_free_inodes;

    int s_ninode;				// first free inode number
    int s_nblock;				// first free block number

};

#define WINIX_MAGIC				0x12345678

#define SUPER_BLOCK_NR			1
#define INODE_MAP_BLOCK_NR		1
#define BLOCK_MAP_BLOCK_NR		1

#endif