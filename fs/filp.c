#include "fs.h"

static struct filp fd_table[NR_FILPS];

int get_fd(struct proc *curr,int start, int *k, struct filp **fpt){
    int i = -1;
    struct filp *f;

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


struct filp *get_filp(int fd){
	if (fd >= 0 && fd < OPEN_MAX) 
		return &fd_table[fd];
	return NULL;
}

struct filp *find_filp(struct inode *ino){
    int i;
    for(i = 0; i< NR_FILPS; i++){
        if(fd_table[i].filp_ino == ino){
            return &fd_table[i];
        }
    }
    return NULL;
}

struct filp *get_free_filp(){
    struct filp* rep;
    for(rep = &fd_table[0]; rep < &fd_table[NR_FILPS]; rep++ ){
        if(rep->filp_ino = NIL_INODE){
            return rep;
        }
    }
	return NULL;
}

void init_filp(){
    struct filp* rep;
    int i = 0;
    for(rep = &fd_table[0]; rep < &fd_table[NR_FILPS]; rep++ ){
        rep->filp_ino = NIL_INODE;
		rep->filp_table_index = i;
        i++;
    }
}
