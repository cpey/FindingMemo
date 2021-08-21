// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <tracer.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "tracer"

static int device_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buf,
	size_t count, loff_t *position)
{
	return count;
}

static ssize_t device_read(struct file *filp, char *buf,
	size_t count, loff_t *position)
{
	return count;
}

static long device_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	switch (cmd) {
	case HOOK_INSTALL:
		break;
	case HOOK_REMOVE:
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.write = device_write,
	.read = device_read,
	.unlocked_ioctl = device_ioctl,
	.release = device_release
};

static int __init tracer_init()
{
	pr_info("Memory Tracer");
	file = debugfs_create_file(DEVICE_NAME, 0200, NULL, NULL, &my_fops);

	return 0;
}

static void __exit tracer_exit()
{
	pr_info("Unloaded Memory Tracer");
}

module_init(tracer_init);
module_exit(tracer_exit);
