// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include "tracer.h"
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/msg.h>


struct msg_msg *(*real_load_msg)(const void __user *src, size_t len);
static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct ftrace_regs *fregs);

unsigned long addr;
static struct ftrace_ops ops __read_mostly = {
	.func = hook_callback,
	.flags = FTRACE_OPS_FL_SAVE_REGS
				| FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED
				| FTRACE_OPS_FL_IPMODIFY
};

// Wrapper to load_msg
struct msg_msg *wr_load_msg(const void __user *src, size_t len)
{
	struct msg_msg *msg;

	pr_info("+ load_msg()\n");
	msg = (*real_load_msg)(src, len);
	pr_info("result: %p\n", msg);

	return msg;
}

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	struct pt_regs *regs = ftrace_get_regs(fregs);
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long) wr_load_msg;
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

void hook_init(unsigned long addr)
{
	// Because kallsyms_lookup_name is no longer exported
	real_load_msg = (void *) addr;
}
