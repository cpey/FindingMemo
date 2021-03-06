// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef TRACER_H_
#define TRACER_H_

#include <asm-generic/compat.h>

struct fname {
	char *name;	
	int len;
};

struct finder_info {
	unsigned long addr;
	struct fname func;
};

struct compat_fname {
	compat_ulong_t name;
	int len;
};

struct compat_finder_info {
	unsigned long addr;
	struct compat_fname func;
};

#define HOOK_IOCTL_NUM		'm'
#define HOOK_INIT		_IO(HOOK_IOCTL_NUM, 0)
#define HOOK_STOP		_IO(HOOK_IOCTL_NUM, 1)
#define HOOK_ADD     		_IOW(HOOK_IOCTL_NUM, 2, struct finder_info)
#define COMPAT_HOOK_ADD     	_IOW(HOOK_IOCTL_NUM, 2, struct compat_finder_info)

#endif /* TRACER_H_ */
