#include "fs.h"

static struct filep fd_table[NR_FILPS];

int get_fd(struct proc *curr,int start, int *k, struct filep **fpt){
    int i = -1;
    struct filep *f;

    for( i=start; i< OPEN_MAX; i++){
        if(current_proc->fp_filp[i] == NIL_FILP){
            *k = i;
            break;
        }
    }

    if(i==-1)
        return ERR;

     for (f = &fd_table[0]; f < &fd_table[NR_FILPS]; f++) {
        if (f->filp_count == 0) {
            // f->filp_mode = bits;
            f->filp_pos = 0L;
            f->filp_flags = 0;
            *fpt = f;
            return(OK);
        }
    }
    return ERR;
}


struct filep *get_filp(int fd){
    return &fd_table[fd];
}

struct filep *find_filp(struct inode *inode){
    int i;
    for(i = 0; i< NR_FILPS; i++){
        if(fd_table[i].filp_ino == inode){
            return &fd_table[i];
        }
    }
    return NULL;
}

struct filep *get_free_filp(){
    struct filep* rep;
    for(rep = &fd_table[0]; rep < &fd_table[NR_FILPS]; rep++ ){
        if(rep->filp_ino = NIL_INODE){
            return rep;
        }
    }
	return NULL;
}

void init_filp(){
    struct filep* rep;
    int i = 0;
    for(rep = &fd_table[0]; rep < &fd_table[NR_FILPS]; rep++ ){
        rep->filp_ino = NIL_INODE;
		rep->filp_table_index = i;
        i++;
    }
}
