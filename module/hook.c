// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include "tracer.h"
#include "hook.h"
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

atomic_t trace_active;

FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src, size_t, len)
{
	struct msg_msg *msg;
	atomic_set(&trace_active, false);
	msg = FM_HOOK_FUNC_NAME(load_msg)(src, len);
	atomic_set(&trace_active, true);
	pr_info("fmemo: load_msg(): msg addr: %px\n", msg);
	return msg;
}

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs)
{
	if (atomic_read(&trace_active))
		regs->ip = (unsigned long) FM_HOOK_WRAP_NAME(load_msg);
}

void hook_init(unsigned long addr)
{
	// Because kallsyms_lookup_name is no longer exported
	FM_HOOK_FUNC_NAME(load_msg) = (void *) addr;
	atomic_set(&trace_active, true);
}

int hook_install(FName* fn)
{
	int err = 0;

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

	err = unregister_ftrace_function(&ops);
	if (err)
		return err;

	err = ftrace_set_filter(&ops, fn->name, fn->len, 0);
	if (err < 0)
		return err;

	return 0;
}
