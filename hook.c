// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/ftrace.h>
#include "tracer.h"

#include <linux/msg.h>  // struct msg_msg

MODULE_LICENSE("GPL");

struct ftrace_ops ops;
unsigned long addr;

struct msg_msg *(*real_load_msg)(const void __user *src, size_t len);
unsigned long (*real_kallsyms_lookup_name)(const char *name);

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs);

void set_ops(void)
{
	ops.func = hook_callback;
	ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;
}

// Wrapper to load_msg
struct msg_msg *wr_load_msg(const void __user *src, size_t len)
{
	struct msg_msg *msg;

	pr_info("load_msg()");
	msg = real_load_msg(src, len);
	pr_info("load_msg() result: %p", msg);

	return msg;
}

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs)
{
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long) wr_load_msg;
}

int hook_install(FName* fn)
{
	int err; 

	*((unsigned long*) real_load_msg) = real_kallsyms_lookup_name(fn->name);
	set_ops();
	err = register_ftrace_function(&ops);
	if (err)
		return err;

	err = ftrace_set_filter(&ops, fn->name, fn->len, 0);
	if (err)
		return err;

	return 0;
}

int hook_remove(FName* fn)
{
	int err; 

	set_ops();
	err = ftrace_set_notrace(&ops, fn->name, fn->len, 0);
	if (err)
		return err;

	err = unregister_ftrace_function(&ops);
	if (err)
		return err;
	
	return 0;
}

void hook_init(unsigned long addr)
{
	*((unsigned long *) real_kallsyms_lookup_name) = addr;
}
