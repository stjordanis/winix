//
// Created by bruce on 6/20/17.
//

#ifndef FS_READ_H
#define FS_READ_H

int sys_read(struct proc* who, int fd, void *buf, int count);
int sys_write(struct proc* who, int fd, void *buf, int count);

#endif // FS_READ_H
