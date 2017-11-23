//
// Created by bruce on 6/20/17.
//

#ifndef FS_OPEN_H
#define FS_OPEN_H

int sys_open(struct proc* who, char *path, int flags, mode_t mode);
#endif // FS_OPEN_H
