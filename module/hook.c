// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include "tracer.h"
#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/msg.h>
#include <linux/slab.h>
#include <linux/syscalls.h>

struct fm_hook_metadata {
	const char *name;
	const void *func;
	const char *rtype;
	const char **types;
	const char **args;
	struct list_head list;
};

LIST_HEAD(fm_hook_list);

#define __FMH_STR_ADECL(t, a)	#a
#define __FMH_STR_TDECL(t, a)	#t
#define __FMH_STR_TRET (t)		#t
#define __MAPR(m, t)			m(t)

#define FM_HOOK_FUNC_DEFINE1(name, ...) FM_HOOK_FUNC_DEFINEx(1, _##name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE2(name, ...) FM_HOOK_FUNC_DEFINEx(2, _##name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE3(name, ...) FM_HOOK_FUNC_DEFINEx(3, _##name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE4(name, ...) FM_HOOK_FUNC_DEFINEx(4, _##name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE5(name, ...) FM_HOOK_FUNC_DEFINEx(5, _##name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE6(name, ...) FM_HOOK_FUNC_DEFINEx(6, _##name, __VA_ARGS__)

#define FM_HOOK_FUNC_DEFINE_MAXARGS  6
#define FM_HOOK_WRAP_DEFINE_MAXARGS  6

#define FM_HOOK_FUNC_DEFINEx(x, sname, ...)				\
	__FM_HOOK_FUNC_DEFINEx(x, sname, __VA_ARGS__)		\
	__FM_HOOK_WRAP_DEFINEx(x, sname, __VA_ARGS__)		\
	__FM_HOOK_META_DEFINEx(x, sname, __VA_ARGS__)

#define __FM_HOOK_FUNC_DEFINEx(x, name, rtype, ...)		\
	rtype (*__do_fm_hook##name)(__MAP(x,__SC_DECL,__VA_ARGS__));

#define __FM_HOOK_WRAP_DEFINEx(x, name, rtype, ...)		\
    rtype (__do_fm_wrap##name)(__MAP(x,__SC_DECL,__VA_ARGS__)) {	\
		rtype t;													\
		pr_info("+ "#name);							\
		t = __do_fm_hook##name(__MAP(x,__SC_ARGS,__VA_ARGS__));		\
		return t; 													\
	}

#define __FM_HOOK_META_DEFINEx(x, sname, srtype, ...) 	\
    static const char *types##sname[] = {				\
        __MAP(x,__FMH_STR_TDECL,__VA_ARGS__)			\
    };													\
    static const char *args##sname[] = {				\
        __MAP(x,__FMH_STR_ADECL,__VA_ARGS__)			\
    };													\
	static struct fm_hook_metadata __used				\
	 __fm_hook_meta##sname = {							\
		.name 	= #sname,								\
		.func 	= __do_fm_wrap##sname,					\
		.rtype	= #srtype, 								\
  		.types 	= x ? types##sname : NULL,				\
  		.args	= x ? args##sname : NULL,				\
  		.list 	= LIST_HEAD_INIT(__fm_hook_meta##sname.list),  \
	};													\
  	list_add(&__fm_hook_meta##name.list, &fm_hook_list);


#define FM_HOOK_FUNC_NAME(name)		__do_fm_hook_##name
#define FM_HOOK_WRAP_NAME(name)		__do_fm_wrap_##name


FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src, size_t, len);

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs);

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
	msg = (*__do_fm_hook_load_msg)(src, len);
	pr_info("result: %p\n", msg);

	return msg;
}

static void notrace hook_callback(unsigned long ip, unsigned long parent_ip,
	struct ftrace_ops *ops, struct pt_regs *regs)
{
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long) FM_HOOK_WRAP_NAME(load_msg);
}

void hook_init(unsigned long addr)
{
	// Because kallsyms_lookup_name is no longer exported
	FM_HOOK_FUNC_NAME(load_msg) = (void *) addr;
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
