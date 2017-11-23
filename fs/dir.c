#include "fs.h"

int init_dir(struct blk_buf* buf) {
	struct direct* dir = (struct direct*)buf->block;
	dir->d_ino = ROOT_INO;
	memcpy(dir->d_name, ".", 2);
	dir++;

	dir->d_ino = ROOT_INO;
	memcpy(dir->d_name, "..", 3);

	return OK;
}
