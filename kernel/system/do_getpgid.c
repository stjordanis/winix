#include <kernel/kernel.h>


int do_getpgid(struct proc* who, struct message* m){
    pid_t pid = m->m1_i1;
    struct proc* to;
    if(pid < 0)
        return EINVAL;
    if(pid == 0)
        pid = who->tgid;

    to = get_proc(pid);
    if(!to)
        return ESRCH;
    m->m1_i1 = to->procgrp;
    return OK;
}

