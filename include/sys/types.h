#ifndef _TYPE_H_
#define _TYPE_H_ 1

#define _CRT_NO_TIME_T 1

#ifndef _SIZE_T
#define	_SIZE_T
typedef unsigned int	size_t;		/* type returned by sizeof */
#endif /* _SIZE_T */

//typedef unsigned char byte;
//typedef unsigned char byte_t;
typedef unsigned int uint32_t;
typedef long clock_t;
typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int off_t;
typedef unsigned int time_t;
typedef unsigned int nlink_t;
typedef int pid_t;
typedef unsigned int loff_t;
typedef int          dev_t;       /* holds (major|minor) device pair */

#ifndef SIGSET_T
#define SIGSET_T
typedef unsigned int sigset_t;
#endif // !SIGSET_T

#endif
