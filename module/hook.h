// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef HOOK_H_
#define HOOK_H_

#include <linux/syscalls.h>

struct fm_hook_metadata {
	const char *name;
	void *func;
	const void *wrap;
	const char *rtype;
	const char **types;
	const char **args;
	struct list_head list;
};

extern struct list_head fm_hooks;

#define for_each_hook(hook, start, end)					\
	for (hook = start;									\
	     (unsigned long)hook < (unsigned long)end;		\
	     hook++)

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

#define FM_HOOK_FUNC_DEFINEx(x, sname, ...)				\
	__FM_HOOK_FUNC_DEFINEx(x, sname, __VA_ARGS__)		\
	__FM_HOOK_WRAP_DECLAREx(x, sname, __VA_ARGS__)		\
	__FM_HOOK_META_DEFINEx(x, sname, __VA_ARGS__)       \
	__FM_HOOK_WRAP_DEFINEx(x, sname, __VA_ARGS__)

#define __FM_HOOK_FUNC_DEFINEx(x, name, rtype, ...)	\
	rtype (*__do_fm_hook##name)(__MAP(x,__SC_DECL,__VA_ARGS__));

#define __FM_HOOK_WRAP_DECLAREx(x, name, rtype, ...) \
	static inline rtype (__do_fm_wrap##name)(__MAP(x,__SC_DECL,__VA_ARGS__));

#define __FM_HOOK_WRAP_DEFINEx(x, name, rtype, ...) \
	static inline rtype (__do_fm_wrap##name)(__MAP(x,__SC_DECL,__VA_ARGS__))

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
		.func 	= &__do_fm_hook##sname,					\
		.wrap 	= __do_fm_wrap##sname,					\
		.rtype	= #srtype, 								\
		.types 	= x ? types##sname : NULL,				\
		.args	= x ? args##sname : NULL,				\
		.list 	= LIST_HEAD_INIT(__fm_hook_meta##sname.list),  \
	};													\
	static struct fm_hook_metadata __used				\
	__attribute__((section("__fm_hooks_metadata")))		\
	*__p_fm_hook_meta##sname = &__fm_hook_meta##sname;

#define FM_HOOK_FUNC_NAME(name)			__do_fm_hook_##name
#define FM_HOOK_WRAP_NAME(name)			__do_fm_wrap_##name

int hook_install(FName*);
int hook_remove(FName*);
void hook_init(unsigned long);

#endif /* HOOK_H_ */
