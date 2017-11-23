#ifndef _MAKEFS_H_
#define _MAKEFS_H_ 1

//16 MB
// #define totalsize (16*1024*1024)

#define TOTAL_SIZE   (16*1024*1024)
extern char disk[TOTAL_SIZE];

int makefs();
#endif