#include "fs.h"

int sys_open(struct proc *who, char *path,int flags, mode_t mode){
    struct filep *filp;
    int i,open_slot;
    int ret;
    struct inode *inode = eat_path(path);


    if(inode == NIL_INODE && mode & O_CREAT){
        inode = alloc_inode(inode->i_dev);
    }

    ret = get_fd(who, 0, &open_slot, &filp);
    who->fp_filp[open_slot] = filp;

    return open_slot;
}



