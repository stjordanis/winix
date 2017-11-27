#ifndef _WINIX_ERR_H_
#define _WINIX_ERR_H_

#include <errno.h>

#define ERR_PTR(p)	((void *)(unsigned short)p)

#define IS_ERR_VALUE(x) ((unsigned int)(void *)(x) >= (unsigned int)_NERROR)

#endif // !_WINIX_ERR_H_
