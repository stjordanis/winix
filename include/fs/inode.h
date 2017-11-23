#ifndef _FS_INODE_H_
#define _FS_INODE_H_ 1

#include <sys/types.h>
#include <fs/const.h>

typedef struct {
  mode_t i_mode;        /* file type, protection, etc. */ //8 bytes
  nlink_t i_nlinks;        /* how many links to this file */ //8bytes
  uid_t i_uid;            /* user id of the file's owner */ //8bytes
  gid_t i_gid;            /* group number */ //8 bytes
  off_t i_size;            /* current file size in bytes */ //8 bytes
  time_t i_atime;        /* time of last access (V2 only) */ //8
  time_t i_mtime;        /* when was file data last changed */ //8
  time_t i_ctime;        /* when was inode itself changed (V2 only)*/ //8
  block_t i_zone[NR_TZONES]; /* zone numbers for direct, ind, and dbl ind */
  
  /* The following items are not present on the disk. */
 
  dev_t i_dev;            /* which device is the inode on */
  ino_t i_num;            /* inode number on its (minor) device */
  int i_count;            /* # times inode used; 0 means slot is free */
  block_t i_ndblock;        /* # direct block, where the inode info is stored in the inode table */
  int i_nindirs;        /* # indirect zones per indirect block */
  struct super_block *i_sp;    /* pointer to super block for inode's device */
  unsigned int flags;
  // char i_dirt;            /* CLEAN or DIRTY */
  // char i_pipe;            /* set to I_PIPE if pipe */
  // char i_mount;            /* this bit is set if file mounted on */
  // char i_seek;            /* set on LSEEK, cleared on READ/WRITE */
  // char i_update;        /* the ATIME, CTIME, and MTIME bits are here */
}inode_t;

extern inode_t inode_table[NR_INODES];

#define NIL_INODE (inode_t *) 0    /* indicates absence of inode slot */
#define I_DIR    1
#define I_PIPE    2
#define I_MOUNT    4
#define I_SEEK    8


void init_inodetable();
inode_t* read_inode(int num);
inode_t* get_inode(int num);
int put_inode(inode_t *inode);
inode_t* alloc_inode();
void init_inode();

#endif
