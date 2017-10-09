/**
 * 
 * WINIX Inter-process Communication.
 *
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * 
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * 
 * @create date 2016-09-19
 * 
*/


#include <sys/ipc.h>
#include <stddef.h>
#include <errno.h>

/**
 * Performs a WRAMP system call.
 *
 * Note: implemented in assembly - wramp_syscall.s
 **/
int wramp_syscall(int operation, ...);


/**
 * Receives a message.
 **/
 int winix_receive(struct message *m) {
    //Note: second parameter is currently unused, but is included to simplify kernel code.
    return wramp_syscall(WINIX_RECEIVE, NULL, m);
}


static int winix_send_comm(int type,int dest, struct message *m){
    int ret = wramp_syscall(type, dest, m);
    if(ret < 0){
        __set_errno(-ret);
        return -1;
    }
    return ret;
}

/**
 * Sends a message to the destination process
 **/
int winix_send(int dest, struct message *m) {
    return winix_send_comm(WINIX_SEND, dest, m);
}

/**
 * Sends and receives a message to/from the destination process.
 *
 * Note: overwrites m with the reply message.
 **/
int winix_sendrec(int dest, struct message *m) {
    return winix_send_comm(WINIX_SENDREC, dest, m);
}
