// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include "tracer.h"
#include "hook.h"

#define DEVICE_NAME "tracer"
#define DRV_VERSION "0.1"

struct dentry *file;

static int device_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buf,
	size_t count, loff_t *position)
{
	return 0;
}

static ssize_t device_read(struct file *filp, char *buf,
	size_t count, loff_t *position)
{
	return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	FName fn;

	switch (cmd) {
		case HOOK_INIT: {
			unsigned long add;
			if (copy_from_user(&add, argp, sizeof(add))) {
				return -EFAULT;
			}

			pr_info("Address to hook: %lX\n", add);
			hook_init(add);
			break;
		}
		case HOOK_INSTALL: {
			int err;

			fn.name = "load_msg";
			fn.len = strlen(fn.name);
			err = hook_install(&fn);
			if (err < 0)
				return err;
			pr_info("Installed hook\n");
			break;
		}
		case HOOK_REMOVE: {
			int err;

			fn.name = "!load_msg";
			fn.len = strlen(fn.name);
			err = hook_remove(&fn);
			if (err < 0)
				return err;
			pr_info("Removed hook\n");
			break;
		}
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

static int __init tracer_init(void)
{
	pr_info("Memory Tracer\n");
	file = debugfs_create_file(DEVICE_NAME, 0200, NULL, NULL, &my_fops);

	return 0;
}

static void __exit tracer_exit(void)
{
	debugfs_remove(file);
	pr_info("Unloaded Memory Tracer\n");
}

module_init(tracer_init);
module_exit(tracer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory tracer");
MODULE_AUTHOR("Carles Pey");
MODULE_VERSION(DRV_VERSION);

