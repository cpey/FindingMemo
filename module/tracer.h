// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef TRACER_H_
#define TRACER_H_

typedef struct FUNCTION_NAME {
	char *name;	
	int len;
} FName;

struct finder_info {
	unsigned long addr;
	FName func;
};

#define HOOK_IOCTL_NUM 'm'
#define HOOK_INSTALL _IOW(HOOK_IOCTL_NUM, 0, FName)
#define HOOK_REMOVE  _IOW(HOOK_IOCTL_NUM, 1, FName)
#define HOOK_INIT    _IOW(HOOK_IOCTL_NUM, 2, struct finder_info)

#endif /* TRACER_H_ */
