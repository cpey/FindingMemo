// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "memo.h"

#define DEVICE 		"tracer"
#define DEBUGFS		"/sys/kernel/debug"

typedef struct FUNCTION_NAME {
	char *name;
	int len;
} FName;

struct finder_info {
	unsigned long addr;
	FName func;
};

struct err_type {
	int _errno;
	char desc[ERR_MAX_LEN];
};

#define HOOK_IOCTL_NUM 'm'
#define HOOK_INSTALL _IOW(HOOK_IOCTL_NUM, 0, FName)
#define HOOK_REMOVE  _IOW(HOOK_IOCTL_NUM, 1, FName)
#define HOOK_ADD     _IOW(HOOK_IOCTL_NUM, 2, struct finder_info)

struct err_type err_info;

unsigned long get_symbol_addr(char *name)
{
	FILE *fd;
	unsigned long addr;
	char dummy, sname[512];
	int ret = 0;

	fd = fopen("/proc/kallsyms", "r");
	if (!fd) {
		return 0;
	}

	while (ret != EOF) {
		ret = fscanf(fd, "%p %c %s\n", (void **) &addr, &dummy, sname);
		if (ret && !strcmp(name, sname)) {
			printf("+ Found symbol %s at 0x%lx\n", name, addr);
			return addr;
		}
	}

	return 0;
}

int add_hook(char *symbol, int fd, struct finder_info *finfo)
{
	int err = 0;

	finfo->func.name = symbol;
	finfo->func.len = strlen(symbol);

	finfo->addr = get_symbol_addr(symbol);
	if (!finfo->addr) {
		_set_err("Symbol %s not found", symbol);
	}

	printf("+ Add hook\n");
	if (ioctl(fd, HOOK_ADD, (void *) &finfo) < 0) {
		_set_err("Error adding hook: %s", strerror(errno));
	}
free:
	return err;
}


int main(int argc, char** argv)
{
	char device[50];
	char err_msg[ERR_MAX_LEN];
	char symbol[SYM_MAX_LEN];
	int fd, err = 0, opt;
	bool symbol_set = false;
	bool remove_hook = false;
	struct finder_info finfo = {0};

	while ((opt = getopt(argc, argv, "s:r")) != -1) {
		switch (opt) {
			case 's':
				strncpy(symbol, optarg, sizeof(symbol));
				symbol_set = true;
				break;
			case 'r':
				remove_hook = true;
				break;
			case '?':
				return EXIT_FAILURE;
		}
	}

	if (sprintf(device, "%s/%s", DEBUGFS, DEVICE) < 0) {
		_exit_err("Sprintf error");
	}

	fd = open(device, O_WRONLY);
	if (fd < 0) {
		_exit_err("Error opening the device: %s", strerror(errno));
	}

	if (remove_hook) {
		printf("+ Remove Hook\n");
		if (ioctl(fd, HOOK_REMOVE, NULL) < 0) {
			_exit_err_free("Hook removal error: %s", strerror(errno));
		}
		goto free;
	}

	if (!symbol_set) {
		if (sprintf(symbol, "%s", "load_msg") < 0) {
			_exit_err_free("Sprintf error");
		}
	}

	if (add_hook(symbol, fd, &finfo)) {
		_exit_err_free(err_info.desc);
	}
	printf("+ Install hook\n");
	if (ioctl(fd, HOOK_INSTALL, NULL) < 0) {
		_exit_err_free("Error installing hook: %s", strerror(errno));
	}

free:
	if (fd)
		if (close(fd))
			_exit_err("Error closing device: %s", strerror(errno));
out:
	if (err)
		perror(err_msg);

	return err;
}
