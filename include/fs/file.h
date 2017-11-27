#ifndef _FS_FILE_H_
#define _FS_FILE_H_ 1

#include <fs/inode.h>
#include <sys/types.h>

#define OPEN_MAX  8

struct filp {
  mode_t filp_mode;				/* RW bits, telling how file is opened */
  int filp_flags;				/* flags from open and fcntl */
  int filp_count;				/* how many file descriptors share this slot?*/
  struct inode *filp_ino;		/* pointer to the inode */
  off_t filp_pos;				/* file position */
  int filp_table_index;
};

struct file_operations {
	dev_t owner;
	loff_t(*llseek) (struct filp *, loff_t, int);
	size_t(*read) (struct filp *, char *, size_t, loff_t *);
	size_t(*write) (struct filp *, const char *, size_t, loff_t *);
	int(*open) (struct inode *, struct filp *);
	int(*close)	(struct inode *, struct filp *);
	int(*ioctl) (struct inode *, struct filp *, unsigned int, unsigned long);
	int(*flush) (struct filp *);
	int(*fsync) (struct filp *, struct dentry *, int datasync);
};

#define NIL_FILP (struct filp *) 0    /* indicates absence of a filp slot */

#endif
