// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define DEVICE 		"tracer"
#define DEBUGFS		"/sys/kernel/debug"

typedef struct FUNCTION_NAME {
	char *name;
	int len;
} FName;

#define HOOK_IOCTL_NUM 0xFF
#define HOOK_INSTALL _IOW(HOOK_IOCTL_NUM, 0, FName)
#define HOOK_REMOVE  _IOW(HOOK_IOCTL_NUM, 1, FName)
#define HOOK_INIT    _IOW(HOOK_IOCTL_NUM, 2, FName)

#define _set_exit_err(msg, ...)					\
	do {										\
		err=1;									\
		sprintf(err_msg, msg, ##__VA_ARGS__);	\
		goto out;								\
	} while (0)

int main(int argc, char** argv)
{
	char device[50];
	char err_msg[200];
	int fd;
	int err;

	if (sprintf(device, "%s/%s", DEBUGFS, DEVICE) < 0) {
		_set_exit_err("Sprintf error\n");
	}

	fd = open(device, O_WRONLY);
	if (fd < 0) {
		_set_exit_err("Error opening the device: %s\n", strerror(errno));
	}

	printf("+ Hook init\n");
	if (ioctl(fd, HOOK_INIT, NULL) < 0) {
		_set_exit_err("Hook init error: %s\n", strerror(errno));
	}

	printf("+ Hook install\n");
	if (ioctl(fd, HOOK_INSTALL, NULL) < 0) {
		_set_exit_err("Hook install error: %s\n", strerror(errno));
	}

out:
	if (err) {
		perror(err_msg);
	}

	return err;

}
