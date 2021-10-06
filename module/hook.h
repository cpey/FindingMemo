// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef HOOK_H_
#define HOOK_H_

#include "tracer.h"
#include <linux/syscalls.h>

#define FM_HOOK_NAME_MAX_LEN 256

struct fm_hook_metadata {
	const char *name;
	void *func;
	const void *wrap;
	const char *rtype;
	const char **types;
	const char **args;
	bool set;
	atomic_t mutex;
	struct list_head list;
};

extern struct list_head fm_hooks;

#define for_each_hook(hook, start, end)			\
	for (hook = start;				\
	     (unsigned long)hook < (unsigned long)end;	\
	     hook++)

#define __FMH_STR_ADECL(t, a)	#a
#define __FMH_STR_TDECL(t, a)	#t
#define __FMH_STR_TRET (t)	#t
#define __MAPR(m, t)		m(t)

#define FM_HOOK_FUNC_DEFINE1(name, ...) FM_HOOK_FUNC_DEFINEx(1, name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE2(name, ...) FM_HOOK_FUNC_DEFINEx(2, name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE3(name, ...) FM_HOOK_FUNC_DEFINEx(3, name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE4(name, ...) FM_HOOK_FUNC_DEFINEx(4, name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE5(name, ...) FM_HOOK_FUNC_DEFINEx(5, name, __VA_ARGS__)
#define FM_HOOK_FUNC_DEFINE6(name, ...) FM_HOOK_FUNC_DEFINEx(6, name, __VA_ARGS__)

#define FM_HOOK_FUNC_DEFINE_MAXARGS  6

#define FM_HOOK_FUNC_DEFINEx(x, sname, ...)		\
	__FM_HOOK_FUNC_DEFINEx(x, sname, __VA_ARGS__)	\
	__FM_HOOK_WRAP_DECLAREx(x, sname, __VA_ARGS__)	\
	__FM_HOOK_META_DEFINEx(x, sname, __VA_ARGS__)  	\
	__FM_HOOK_WRAP_DEFINEx(x, sname, __VA_ARGS__)

#define __FM_HOOK_FUNC_DEFINEx(x, name, rtype, ...)			\
	rtype (*__do_fm_hook_##name)(__MAP(x,__SC_DECL,__VA_ARGS__)); 	\
	typedef rtype (*type_##name)(__MAP(x,__SC_DECL,__VA_ARGS__));

#define __FM_HOOK_WRAP_DECLAREx(x, name, rtype, ...) 	\
	static inline rtype (__do_fm_wrap_##name)(__MAP(x,__SC_DECL,__VA_ARGS__));

#define __FM_HOOK_WRAP_DEFINEx(x, name, rtype, ...) 	\
	static inline rtype (__do_fm_wrap_##name)(__MAP(x,__SC_DECL,__VA_ARGS__))

#define __FM_HOOK_META_DEFINEx(x, sname, srtype, ...) 			\
	static const char *types_##sname[] = {				\
		__MAP(x,__FMH_STR_TDECL,__VA_ARGS__)			\
	};								\
	static const char *args_##sname[] = {				\
		__MAP(x,__FMH_STR_ADECL,__VA_ARGS__)			\
	};								\
	static struct fm_hook_metadata __used				\
	 __fm_hook_meta_##sname = {					\
		.name 	= #sname,					\
		.func 	= &__do_fm_hook_##sname,			\
		.wrap 	= __do_fm_wrap_##sname,				\
		.rtype	= #srtype, 					\
		.types 	= x ? types_##sname : NULL,			\
		.args	= x ? args_##sname : NULL,			\
		.set 	= false,                                        \
		.list 	= LIST_HEAD_INIT(__fm_hook_meta_##sname.list),  \
	};								\
	static struct fm_hook_metadata __used				\
	__attribute__((section("__fm_hooks_metadata")))			\
	*__p_fm_hook_meta_##sname = &__fm_hook_meta_##sname;

#define FM_HOOK_FUNC_PTR(name)	((type_##name) curr_hook->func)
#define FM_HOOK_FUNC		curr_hook->func
#define FM_HOOK_WRAP		curr_hook->wrap

int hook_init(void);
int hook_stop(void);
int hook_add(struct finder_info *);
int hook_remove(struct finder_info *);
#endif /* HOOK_H_ */
