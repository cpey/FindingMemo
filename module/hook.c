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

FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src,
		size_t, len)
{
	struct msg_msg *msg;
	atomic_set(&curr_hook->mutex, false);
	msg = FM_HOOK_FUNC_PTR(load_msg)(src, len);
	atomic_set(&curr_hook->mutex, true);
	pr_info("fmemo: load_msg(): msg addr: %px\n", msg);
	return msg;
}

FM_HOOK_FUNC_DEFINE1(free_msg, void, struct msg_msg *, msg)
{
	atomic_set(&curr_hook->mutex, false);
	FM_HOOK_FUNC_PTR(free_msg)(msg);
	atomic_set(&curr_hook->mutex, true);
	pr_info("fmemo: free_msg(): msg addr: %px\n", msg);
}

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
	atomic_set(&curr_hook->mutex, true);
	return 0;
}

int hook_install(FName* fn)
{
	int err = 0;

	if (hook_installed)
		return -EALREADY;

	err = ftrace_set_filter(&ops, fn->name, fn->len, 1);
	if (err < 0)
		return err;

	err = register_ftrace_function(&ops);
	if (err < 0)
		return err;

	return 0;
}

int hook_remove(FName* fn)
{
	int err; 

	if (!hook_installed)
		return -EALREADY;

	err = unregister_ftrace_function(&ops);
	if (err)
		return err;

	err = ftrace_set_filter(&ops, fn->name, fn->len, 0);
	if (err < 0)
		return err;

	return 0;
}
