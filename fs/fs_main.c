#include "fs.h"
#include <fs/makefs.h>
#include <fs/inode.h>
#include <stdio.h>

struct proc pcurrent_proc;
struct proc *current_proc;
struct super_block *sb;



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
    
    current_proc = &pcurrent_proc;
    current_proc->fp_workdir = current_proc->fp_rootdir = get_inode(&dev, ROOT_INO);
    init_fs(&dev.sb);


    char c = 'a';
    int fd = sys_open(current_proc, "/foo.txt",O_CREAT, O_RDWR);
    for (i = 0; i < 2046; i++) {
        sys_write(current_proc, fd, &c, 1);
        c++;
        if (c == 'z')
            c = 'a';
    }
    sys_write(current_proc,fd, "a", 2);
    sys_close(current_proc, fd);

    char buf[2048];
    fd = sys_open(current_proc, "/foo.txt", 0 ,O_RDONLY);
    sys_read(current_proc, fd,buf, 2048);
    sys_close(current_proc, fd);

    printf("\nread foo.txt\n");
    printf("Got \"%s\" from foo.txt\n",buf);

}