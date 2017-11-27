#include "fs.h"


int sys_open(struct proc *who, char *path, int flags, mode_t mode) {
	struct filp *filp;
	int i, open_slot;
	int ret;
	char string[DIRSIZ];
	struct inode *parent_ino = last_dir(path, string);
	struct inode *ino = advance(parent_ino, string);

	if (ino == NULL && flags & O_CREAT) {
		ino = new_inode_in_path(parent_ino->i_dev, path);
	}

	ret = get_fd(who, 0, &open_slot, &filp);
	filp->filp_ino = ino;
	filp->filp_mode = mode;
	filp->filp_flags = flags;

	if (flags & O_APPEND) {
		filp->filp_pos = ino->i_size;
	}

	who->fp_filp[open_slot] = filp;

	return open_slot;
}

 int sys_close(struct proc *who,int fd){

     struct filp *filp = who->fp_filp[fd];
     filp->filp_ino = NIL_INODE;
     filp->filp_pos = 0;

     who->fp_filp[fd] = 0;
     return OK;
 }


