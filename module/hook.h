// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef HOOK_H_
#define HOOK_H_

#include <linux/syscalls.h>
#include "tracer.h"

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
extern struct fm_hook_metadata *curr_hook;

#define for_each_section_elem(hook, start, end)		\
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
	__FM_HOOK_META_DEFINEx(x, sname, __VA_ARGS__)	\
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

/*
 * Sysfs attribute definition
 */
struct fm_hook_attr {
	const char *name;
	struct kobj_attribute *attr;
	bool set;
	bool active;
	struct list_head list;
};

extern struct list_head fm_attrs;

#define FM_HOOK_ATTR_DEFINE(name)					\
	__FM_HOOK_ATTR_DECLARE(name)					\
	__FM_HOOK_ATTR_ATTR(name)					\
	__FM_HOOK_ATTR_META(name)					\
	__FM_HOOK_ATTR_DEFINE(name)

#define __FM_HOOK_ATTR_DECLARE(sname)					\
	static ssize_t fm_show_##sname(struct kobject *kobj,		\
			struct kobj_attribute *attr, char *buf);	\

#define __FM_HOOK_ATTR_ATTR(sname)					\
	struct kobj_attribute fm_hook_attr_##sname =			\
		__ATTR(fm_##sname, S_IRUGO, fm_show_##sname, NULL);

#define __FM_HOOK_ATTR_META(sname)					\
	static struct fm_hook_attr __used				\
	 __fm_hook_attr_##sname = {					\
		.name 	= #sname,					\
		.attr 	= &fm_hook_attr_##sname,			\
		.set 	= false,					\
		.active = false,					\
		.list 	= LIST_HEAD_INIT(__fm_hook_attr_##sname.list),	\
	};								\
	static struct fm_hook_attr __used				\
	__attribute__((section("__fm_hooks_attr")))			\
	*__p_fm_hook_attr_##sname = &__fm_hook_attr_##sname;

#define __FM_HOOK_ATTR_DEFINE(sname)					\
	static ssize_t fm_show_##sname(struct kobject *kobj,		\
			struct kobj_attribute *attr, char *buf)

#define FM_HOOK_ATTR(name)	fm_hook_attr_##name

int hook_init(void);
int hook_stop(void);
int hook_add(struct finder_info *);
int create_sysfs_show_dir(void);
void remove_sysfs_show_dir(void);
#endif /* HOOK_H_ */
