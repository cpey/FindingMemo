// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef TRACER_H_
#define TRACER_H_

typedef struct FUNCTION_NAME {
	char *name;	
	int len;
} FName;

#define HOOK_IOCTL_NUM 0xFF
#define HOOK_INSTALL _IOW(HOOK_IOCTL_NUM, 0, FName)
#define HOOK_REMOVE  _IOW(HOOK_IOCTL_NUM, 1, FName)
#define HOOK_INIT    _IOW(HOOK_IOCTL_NUM, 2, __u64)

#endif /* TRACER_H_ */
