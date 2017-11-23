#ifndef _WINIX_DEVICE_H_
#define	_WINIX_DEVICE_H_

#include <sys/types.h>
#include <fs/file.h>
#include <fs/super.h>


struct device_driver {
	dev_t owner;
	int(*probe) (struct device *dev);
	void(*shutdown) (struct device *dev);
};

struct device {
	dev_t dev;
	char *name;
	struct device_driver* driver;
	const struct file_operations *ops;
	//struct list_head list;
	unsigned int count;
};

struct bdev {
	dev_t dev;
	char *name;
	struct device_driver* driver;
	const struct file_operations *ops;
	//struct list_head list;
	struct super_block sb;
	//unsigned int count;
};

#endif
