#ifndef _FS_PATH_H_
#define _FS_PATH_H_ 1

// each direct occupies 32 bytes, with 8 bytes for d_ino, and 24 bytes for directory name
#ifndef DIRSIZ
#define DIRSIZ    32
#endif

#define NAME_MAX    32
#define PATH_MAX    128

char *get_name(char *old_name, char string[NAME_MAX]);
struct inode *advance(struct inode *dirp, char string[NAME_MAX]);
struct inode *last_dir(char *path, char string[DIRSIZ]);
struct inode* eat_path(char *path);

#endif