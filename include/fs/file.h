#ifndef _FS_FILE_H_
#define _FS_FILE_H_ 1

#include <fs/inode.h>
#include <sys/types.h>

#define OPEN_MAX  8

struct file_operations {
	dev_t owner;
	loff_t(*llseek) (struct filep *, loff_t, int);
	size_t(*read) (struct filep *, char *, size_t, loff_t *);
	size_t(*write) (struct filep *, const char *, size_t, loff_t *);
	int(*open) (struct inode *, struct filep *);
	int(*close)	(struct inode *, struct filep *);
};

struct filep {
  mode_t filp_mode;        /* RW bits, telling how file is opened */
  int filp_flags;        /* flags from open and fcntl */
  int filp_count;        /* how many file descriptors share this slot?*/
  struct inode *filp_ino;    /* pointer to the inode */
  off_t filp_pos;        /* file position */

  /* following are for fd-type-specific select() */
  // int filp_pipe_select_ops;

   int filp_table_index;
};

#define NIL_FILP (struct filep *) 0    /* indicates absence of a filp slot */

#endif
