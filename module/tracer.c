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
#include "hook.h"
#include "tracer.h"

#define DEVICE_NAME "tracer"
#define DRV_VERSION "0.1"

DEFINE_MUTEX(finder_mutex);

struct tracer_info {
	bool hook_initiated;
	bool added_hooks_metadata;
};

struct tracer_info _tracer_info = { false, false };
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

static int tracer_hook_stop(void)
{
	int err;

	err = hook_stop();
	if (!err)
		pr_info("Function hooking stopped\n");

	return err;
}

static long device_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case HOOK_ADD: {
		struct finder_info finfo;
		int err;

		if (copy_from_user(&finfo, argp, sizeof(struct finder_info))) {
			return -EFAULT;
		}

		pr_info("Hooking %lx (%s)\n", finfo.addr, finfo.func.name);
		err = hook_add(&finfo);
		if (err < 0) {
			return err;
		}
		break;
	}
	case HOOK_INIT: {
		int err;

		if (_tracer_info.hook_initiated) {
			return -EFAULT;
		}

		err = hook_init();
		if (err < 0)
			return err;

		pr_info("Function hooking initiated\n");
		_tracer_info.hook_initiated = true;
		break;
	}
	case HOOK_STOP: {
		int err;

		if (!_tracer_info.hook_initiated) {
			return -EFAULT;
		}

		err = tracer_hook_stop();
		if (err < 0) {
			return err;
		}

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

static const struct file_operations tracer_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.write = device_write,
	.read = device_read,
	.unlocked_ioctl = device_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =  compat_ptr_ioctl,
#endif /* CONFIG_COMPAT */
	.release = device_release
};

static int __register_hook(struct fm_hook_metadata *hook)
{
	list_add(&hook->list, &fm_hooks);
	return 0;
}

static int __register_attr(struct fm_hook_attr *attr)
{
	list_add(&attr->list, &fm_attrs);
	return 0;
}

static int __remove_hook(struct fm_hook_metadata *hook)
{
	list_del(&hook->list);
	return 0;
}

static int __remove_attr(struct fm_hook_attr *attr)
{
	list_del(&attr->list);
	return 0;
}

static void finder_module_add_hooks(struct module *mod)
{
	struct fm_hook_metadata **hook, **start, **end;
	if (!mod->num_finder_hooks)
		return;

	start = mod->finder_hooks;
	end = mod->finder_hooks + mod->num_finder_hooks;

	for_each_section_elem(hook, start, end) {
		__register_hook(*hook);
	}
}

static void finder_module_add_attrs(struct module *mod)
{
	struct fm_hook_attr **attr, **start, **end;
	if (!mod->num_finder_attrs)
		return;

	start = mod->finder_attrs;
	end = mod->finder_attrs + mod->num_finder_attrs;

	for_each_section_elem(attr, start, end) {
		__register_attr(*attr);
	}
}

static void finder_module_remove_hooks(void)
{
	struct fm_hook_metadata *hook, *p;

	list_for_each_entry_safe(hook, p, &fm_hooks, list) {
		__remove_hook(hook);
	}
}

static void finder_module_remove_attrs(void)
{
	struct fm_hook_attr *attr, *p;

	list_for_each_entry_safe(attr, p, &fm_attrs, list) {
		__remove_attr(attr);
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
		finder_module_add_attrs(mod);
		_tracer_info.added_hooks_metadata = true;
		break;
	case MODULE_STATE_GOING:
		finder_module_remove_hooks();
		finder_module_remove_attrs();
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
	file = debugfs_create_file(DEVICE_NAME, 0222, NULL, NULL, &tracer_fops);

	ret = register_module_notifier(&finder_module_nb);
	if (ret) {
		pr_warn("Failed to register hook metadata module notifier\n");
		return ret;
	}

	ret = create_sysfs_show_dir();
	if (ret)
		return ret;

	return 0;
}

static void __exit tracer_exit(void)
{
	int ret;

	if (_tracer_info.hook_initiated) {
		tracer_hook_stop();
		_tracer_info.hook_initiated = false;
	}

	if (_tracer_info.added_hooks_metadata) {
		finder_module_remove_hooks();
		finder_module_remove_attrs();
	}

	ret = unregister_module_notifier(&finder_module_nb);
	if (ret) {
		pr_warn("Failed to unregister hook metadata module notifier\n");
	}

	remove_sysfs_show_dir();
	debugfs_remove(file);
	pr_info("Unloaded Memory Tracer\n");
}

module_init(tracer_init);
module_exit(tracer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory tracer");
MODULE_AUTHOR("Carles Pey");
MODULE_VERSION(DRV_VERSION);

