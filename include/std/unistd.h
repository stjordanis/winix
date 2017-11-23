#ifndef _UNISTD_H_
#define _UNISTD_H_ 1

#ifndef _POSIX_SOURCE
#define _POSIX_ 1
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/unistd.h>
// #include <sys/direct.h>
// #include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);


#endif
