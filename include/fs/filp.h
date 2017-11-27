#ifndef _FS_FILP_H_
#define _FS_FILP_H_ 1

#include <kernel/proc.h>
#include <fs/file.h>

int get_fd(struct proc *curr,int start, int *k, struct filp **fpt);
struct filp *get_filp(int fd);
struct filp *find_filp(struct inode *inode);
struct filp *get_free_filp();
void init_filp();

#endif