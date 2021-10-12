// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include "tracer.h"
#include "hook.h"
#include "types.h"
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/msg.h>
#include <linux/slab.h>

LIST_HEAD(fm_hooks);
LIST_HEAD(fm_attrs);

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

int hook_add(struct finder_info *finfo)
{
	int err;

	err = set_current_hook(finfo->func.name);
	if (err < 0)
		return -ENOMSG;

	FM_HOOK_FUNC = (void *) finfo->addr;
	curr_hook->set = true;
	atomic_set(&curr_hook->mutex, true);
	return 0;
}

int hook_remove(struct finder_info *finfo)
{
       int err;

       err = set_current_hook(finfo->func.name);
       if (err < 0)
               return -ENOMSG;

       curr_hook->set = false;
       atomic_set(&curr_hook->mutex, false);
       return 0;
}

int hook_init()
{
	int err = 0;
	struct fm_hook_metadata *hook;
	struct fm_hook_attr *sysfs;
	struct kobject mod_kobj;

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

	mod_kobj = (((struct module *)(THIS_MODULE))->mkobj).kobj;
	list_for_each_entry(sysfs, &fm_attrs, list) {
		err = sysfs_create_file(&mod_kobj, &sysfs->attr->attr);
		if (err) {
			pr_warn("Failed to create sysfs file\n");
			return err;
		}
	}

	err = register_ftrace_function(&ops);
	if (err < 0)
		return err;

	hook_installed = true;
	return 0;
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
	}

	hook_installed = false;
	return 0;
}
