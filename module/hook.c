// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/msg.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include "tracer.h"
#include "hook.h"
#include "types.h"

#define FM_SYSFS_SHOW_DIR "show"

LIST_HEAD(fm_hooks);
LIST_HEAD(fm_attrs);
struct kobject *show_kobj;

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs);

static struct ftrace_ops ops __read_mostly = {
	.func = hook_callback,
	.flags = FTRACE_OPS_FL_SAVE_REGS
				| FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED
				| FTRACE_OPS_FL_IPMODIFY
};

struct fm_hook_metadata *curr_hook;
static bool hook_installed = false;

static bool set_current_hook(char *name)
{
	struct fm_hook_metadata *hook;
	int err = -1;

	list_for_each_entry(hook, &fm_hooks, list) {
		if (!strcmp(hook->name, name))  {
			curr_hook = hook;
			err = 0;
		}
	}

	return err;
}
static bool set_current_hook_ip(unsigned long ip)
{
	struct fm_hook_metadata *hook;
	int err = -1;

	list_for_each_entry(hook, &fm_hooks, list) {
		if (hook->func == (void *) ip)  {
			curr_hook = hook;
			err = 0;
		}
	}

	return err;
}

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs)
{
	set_current_hook_ip(ip);
	if (atomic_read(&curr_hook->mutex))
		regs->ip = (unsigned long) FM_HOOK_WRAP;
}

int create_sysfs_show_dir()
{
	struct kobject mod_kobj;

	mod_kobj = (((struct module *)(THIS_MODULE))->mkobj).kobj;
	show_kobj = kobject_create_and_add(FM_SYSFS_SHOW_DIR, &mod_kobj);
	if (!show_kobj) {
		pr_info("kobject_create_and_add failed\n");
		return -EINVAL;
	}

	return 0;
}

static inline void remove_sysfs_attrs(void)
{
	struct fm_hook_attr *sysfs;

	list_for_each_entry(sysfs, &fm_attrs, list) {
		if (sysfs->active) {
			sysfs_remove_file(show_kobj, &sysfs->attr->attr);
			kobject_put(show_kobj);
			sysfs->active = false;
		}
	}
}

inline void remove_sysfs_show_dir()
{
	if (hook_installed) {
		remove_sysfs_attrs();
		hook_installed = false;
		kobject_put(show_kobj);
	}
}

int hook_add(struct finder_info *finfo)
{
	int err;
	struct fm_hook_attr *sysfs;

	err = set_current_hook(finfo->func.name);
	if (err < 0)
		return -ENOMSG;
	FM_HOOK_FUNC = (void *) finfo->addr;
	curr_hook->set = true;
	atomic_set(&curr_hook->mutex, true);

	list_for_each_entry(sysfs, &fm_attrs, list) {
		if (!strncmp(sysfs->name, finfo->func.name, finfo->func.len))
			sysfs->set = true;
	}
	return 0;
}

int hook_init()
{
	int err = 0;
	struct fm_hook_metadata *hook;
	struct fm_hook_attr *sysfs;

	if (hook_installed)
		return -EALREADY;

	list_for_each_entry(hook, &fm_hooks, list) {
		if (!hook->set)
			continue;
		err = ftrace_set_filter(&ops, (unsigned char *) hook->name,
		                        strlen(hook->name), 0);
		if (err < 0)
			return err;
	}

	list_for_each_entry(sysfs, &fm_attrs, list) {
		if (!sysfs->set)
			continue;
		err = sysfs_create_file(show_kobj, &sysfs->attr->attr);
		if (err) {
			pr_warn("Failed to create sysfs file\n");
			goto err_sysfs;
		}
		kobject_get(show_kobj);
		sysfs->active = true;
	}

	err = register_ftrace_function(&ops);
	if (err < 0)
		goto err_sysfs;

	hook_installed = true;
	return 0;

err_sysfs:
	remove_sysfs_attrs();
	return -EFAULT;
}

int hook_stop()
{
	int err; 
	int len;
	char func[FM_HOOK_NAME_MAX_LEN];
	struct fm_hook_metadata *hook;

	if (!hook_installed)
		return -EALREADY;

	err = unregister_ftrace_function(&ops);
	if (err)
		return err;

	list_for_each_entry(hook, &fm_hooks, list) {
		if (!hook->set)
			continue;

		len = snprintf(func, FM_HOOK_NAME_MAX_LEN, "!%s", hook->name);
		if (len < 0)
			return len;

		err = ftrace_set_filter(&ops, func, len, 0);
		if (err < 0)
			return err;

		hook->set = false;
	}

	remove_sysfs_attrs();
	hook_installed = false;
	return 0;
}
