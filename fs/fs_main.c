#include "fs.h"
#include <fs/makefs.h>
#include <fs/inode.h>
#include <stdio.h>

struct proc pcurrent_proc;
struct proc *current_proc;
struct super_block *sb;

/*	Blocks are arranged as below
	0 - 1		boot sector
	1 - 2		super block sector
	2 - 3		block bitmap 
	3 - 4		inode bitmap
	4 - 132		inode table
	132 - 16384	free blocks
*/



void init_fs(struct super_block* sb) {

    printf("\nNum of blocks in use %d\n", sb->s_block_inuse);
    printf("Block size %d\n inode size %d\n", sb->s_block_size, sb->s_inode_size);
    printf("inode per block %d\n", sb->s_inode_per_block);
}

int fs_main(){
	struct bdev dev;
    struct inode *rootinode;
    int i;

	init_buf();
	init_inode();
	init_filp();

	makefs(&dev);
    

    /*sb->s_iroot = get_inode(1);
    sb->s_iroot->i_nlinks += 1*/
    current_proc = &pcurrent_proc;
    current_proc->fp_workdir = current_proc->fp_rootdir = get_inode(&dev, ROOT_INO);
    init_fs(&dev.sb);


    char abc[] = "abcdefghijklmnopqrstuvwxyz";
    char c = 'a';
    int fd = sys_open(current_proc, "/foo.txt",O_CREAT, O_RDWR);
    for (i = 0; i < 2048; i++) {
        sys_write(current_proc, fd, &c, 1);
        c++;
        if (c == 'z')
            c = 'a';
    }
    sys_write(current_proc,fd, "a", 2);
    sys_close(current_proc, fd);

    char buf[1024];
    fd = sys_open(current_proc, "/foo.txt", 0 ,O_RDONLY);
    sys_read(current_proc, fd,buf,1024);
    sys_close(current_proc, fd);

    printf("\nread foo.txt\n");
    printf("Got \"%s\" from foo.txt\n",buf);

}