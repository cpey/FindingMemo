// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <linux/ftrace.c>
#include <tracer.h>

struct ftrace_ops ops = {
	.func = callback_func,
	.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;
}

int hook_install(FName* fn) {
	int err; 

	err = register_ftrace_function(&ops);
	if (err)
		return err;

	err = ftrace_set_filter(ops, fn->name, fn->len, 0);
	if (err)
		return err;

	return 0;
}

int hook_remove(FName fn) {
	int err; 

	err = ftrace_set_notrace(ops, fn->name, fn->len, 0);
	if (err)
		return err;

	err = unregister_ftrace_function(&ops);
	if (err)
		return err;
	
	return 0;
}
