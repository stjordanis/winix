/**
 * Syscall in this file: kill
 * Input:    m1_i1: pid to which the signal is sent
 *
 * Return:     reply_res:    default return
 * 
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * 
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * 
 * @create date 2017-08-23 06:09:49
 * 
*/
#include <kernel/kernel.h>

int do_kill(struct proc *who, struct message *m){
    struct proc *to;
    int valid_targets = 0;
    pid_t pid = m->m1_i1, pgid;
    int signum = m->m1_i2;

    if(signum < 0 || signum >= _NSIG)
        return EINVAL;

    if(pid == 1 && (signum == SIGSTOP || signum == SIGKILL))
        return EINVAL;

    foreach_proc(to){
        if(pid < -1 && -pid != to->procgrp) continue;
        if(pid == 0 && to->procgrp == who->procgrp) continue;
        if(pid > 0 && pid != to->tgid)   continue;
        if(pid == -1 && to->tgid == 1)   continue;

        if(signum == 0)
            return OK;

        /*
         * if the process to which we are sending 
         * is blocked, we will need to temporarily unblock it
         * so that the scheduler will trigger the signal handling
         */
        
        send_sig(to, signum);
        if(to != who && to->state)
            handle_pendingsig(to);
        
        
	    valid_targets++;
    }

    if(!valid_targets)
	    return ESRCH;

    return OK;
}
