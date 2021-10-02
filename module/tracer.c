// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include "tracer.h"
#include "hook.h"

#define DEVICE_NAME "tracer"
#define DRV_VERSION "0.1"

DEFINE_MUTEX(finder_mutex);

typedef struct tracer_info {
	bool hook_set;
	bool hook_initiated;
	bool added_hooks_metadata;
} TRACER_INFO;

struct tracer_info _tracer_info = { false, false, false };
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

static int tracer_hook_install(void)
{
	int err;
	FName fn;

	fn.name = "load_msg";
	fn.len = strlen(fn.name);
	err = hook_install(&fn);
	if (err < 0)
		return err;
	pr_info("Installed hook\n");
	return err;
}

static int tracer_hook_remove(void) 
{
	int err;
	FName fn;

	fn.name = "!load_msg";
	fn.len = strlen(fn.name);
	err = hook_remove(&fn);
	if (!err)
		pr_info("Removed hook\n");
	return err;
}

static long device_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	switch (cmd) {
		case HOOK_INIT: {
			unsigned long add;

			if (_tracer_info.hook_initiated) {
				return -EFAULT;
			}

			if (copy_from_user(&add, argp, sizeof(add))) {
				return -EFAULT;
			}

			pr_info("Address to hook: %lX\n", add);
			hook_init(add);
			_tracer_info.hook_initiated = true;
			break;
		}
		case HOOK_INSTALL: {
			int err;

			if (_tracer_info.hook_set) {
				return -EFAULT;		
			}

			err = tracer_hook_install();
			if (err < 0) {
				return err;
			}

			_tracer_info.hook_set = true;
			break;
		}
		case HOOK_REMOVE: {
			int err;

			if (!_tracer_info.hook_set) {
				return -EFAULT;		
			}

			err = tracer_hook_remove();
			if (err < 0) {
				return err;
			}

			_tracer_info.hook_set = false;
			_tracer_info.hook_initiated = false;
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

static int __register_hook(struct fm_hook_metadata *hook)
{
	list_add(&hook->list, &fm_hooks);
	return 0;
}

static int __remove_hook(struct fm_hook_metadata *hook)
{
	list_del(&hook->list);
	return 0;
}

static void finder_module_add_hooks(struct module *mod)
{
	struct fm_hook_metadata **hook, **start, **end;
	if (!mod->num_finder_hooks)
		return;

	start = mod->finder_hooks;
	end = mod->finder_hooks + mod->num_finder_hooks;

	for_each_hook(hook, start, end) {
		__register_hook(*hook);
	}
}

static void finder_module_remove_hooks(void)
{
	struct fm_hook_metadata *hook, *p;

	list_for_each_entry_safe(hook, p, &fm_hooks, list) {
		__remove_hook(hook);
	}
}

static int finder_module_notify(struct notifier_block *self,
			       unsigned long val, void *data)
{
	struct module *mod = data;

	mutex_lock(&finder_mutex);
	switch (val) {
	// MODULE_STATE_LIVE to add hooks when the module is loaded
	case MODULE_STATE_LIVE:
	case MODULE_STATE_COMING:
		finder_module_add_hooks(mod);
		_tracer_info.added_hooks_metadata = true;
		break;
	case MODULE_STATE_GOING:
		finder_module_remove_hooks();
		_tracer_info.added_hooks_metadata = false;
		break;
	}
	mutex_unlock(&finder_mutex);

	return 0;
}

static struct notifier_block finder_module_nb = {
	.notifier_call = finder_module_notify,
};

static int __init tracer_init(void)
{
	int ret;

	pr_info("Memory Tracer\n");
	file = debugfs_create_file(DEVICE_NAME, 0200, NULL, NULL, &my_fops);
	ret = register_module_notifier(&finder_module_nb);
	if (ret)
		pr_warn("Failed to register hook metadata module notifier\n");

	return 0;
}

static void __exit tracer_exit(void)
{
	int ret;

	if (_tracer_info.hook_set) {
		tracer_hook_remove();
		_tracer_info.hook_set = false;
	}
	if (_tracer_info.added_hooks_metadata)
		finder_module_remove_hooks();
	ret = unregister_module_notifier(&finder_module_nb);
	if (ret)
		pr_warn("Failed to unregister hook metadata module notifier\n");
	debugfs_remove(file);
	pr_info("Unloaded Memory Tracer\n");
}

module_init(tracer_init);
module_exit(tracer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory tracer");
MODULE_AUTHOR("Carles Pey");
MODULE_VERSION(DRV_VERSION);

