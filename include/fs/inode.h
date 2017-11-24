#ifndef _FS_INODE_H_
#define _FS_INODE_H_ 1

#include <sys/types.h>
#include <fs/const.h>

struct inode {
  mode_t i_mode;        /* file type, protection, etc. */ //4 bytes
  nlink_t i_nlinks;        /* how many links to this file */ //4bytes
  uid_t i_uid;            /* user id of the file's owner */ //4bytes
  gid_t i_gid;            /* group number */ //4 bytes
  off_t i_size;            /* current file size in bytes */ //4 bytes
  time_t i_atime;        /* time of last access (V2 only) */ //4
  time_t i_mtime;        /* when was file data last changed */ //4
  time_t i_ctime;        /* when was inode itself changed (V2 only)*/ //4
  block_t i_zone[NR_TZONES]; /* zone numbers for direct, ind, and dbl ind */
  
  /* The following items are not present on the disk. */
 
  struct bdev* i_dev;            /* which device is the inode on */
  ino_t i_num;            /* inode number on its (minor) device */
  int i_count;            /* # times inode used; 0 means slot is free */
  block_t i_ndblock;        /* # direct block, where the inode info is stored in the inode table */
  int i_nindirs;        /* # indirect zones per indirect block */
  struct super_block *i_sp;    /* pointer to super block for inode's device */
  unsigned int flags;
  char i_dirt;            /* CLEAN or DIRTY */
  char i_pipe;            /* set to I_PIPE if pipe */
  // char i_mount;            /* this bit is set if file mounted on */
  // char i_seek;            /* set on LSEEK, cleared on READ/WRITE */
  // char i_update;        /* the ATIME, CTIME, and MTIME bits are here */
};

extern struct inode inodeable[NR_INODES];

#define NIL_INODE (struct inode *) 0    /* indicates absence of inode slot */
#define I_DIR    1
#define I_PIPE    2
#define I_MOUNT    4
#define I_SEEK    8

#define DISK_INODE_SIZ	offsetof(struct inode, i_dev)

void init_inodetable();
struct inode* read_inode(struct bdev* dev, int num);
struct inode* get_inode(struct bdev* dev, int num);
int put_inode(struct bdev* dev, struct inode *inode);
struct inode* alloc_inode(struct bdev* dev);
void init_inode();
struct inode* new_inode_in_path(struct bdev* dev, char* path);

#endif
