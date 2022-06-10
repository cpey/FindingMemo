// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Carles Pey <cpey@pm.me>
 */

#ifndef TEST_H_
#define TEST_H_

#include <sys/ioctl.h>

#define DEVICE			"tracer"
#define DEBUGFS			"/sys/kernel/debug"

#define HOOK_IOCTL_NUM 	'm'
#define HOOK_INIT	_IO(HOOK_IOCTL_NUM, 0)
#define HOOK_STOP	_IO(HOOK_IOCTL_NUM, 1)
#define HOOK_ADD	_IOW(HOOK_IOCTL_NUM, 2, struct finder_info)

#define ERR_MAX_LEN 	256
#define SYM_MAX_LEN 	256
#define LINE_MAX_LEN	512

struct fname {
	char *name;
	int len;
};

struct finder_info {
#if X86_32
	unsigned long addr[2];
#else
	unsigned long addr[1];
#endif
	struct fname func;
};

struct err_type {
	int _errno;
	char desc[ERR_MAX_LEN];
};

extern struct err_type err_info;

#define _set_err(msg, ...) \
	do { \
		err = 1; \
		sprintf(err_info.desc, msg, ##__VA_ARGS__); \
		err_info._errno = errno; \
		goto free; \
	} while (0)

#define _exit_err_free(msg, ...) \
	do { \
		err = 1; \
		sprintf(err_msg, msg, ##__VA_ARGS__); \
		goto free; \
	} while (0)

#define _exit_err(msg, ...) \
	do { \
		err = 1; \
		sprintf(err_msg, msg, ##__VA_ARGS__); \
		goto out; \
	} while (0)

#define _exit_err_errno(_err, msg, ...) \
	do { \
		err = 1; \
		errno = _err; \
		sprintf(err_msg, msg, ##__VA_ARGS__); \
		goto out; \
	} while (0)

typedef enum {false, true} bool;

#endif /* TEST_H_ */
