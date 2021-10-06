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

#define DEVICE		"tracer"
#define DEBUGFS		"/sys/kernel/debug"
#define LINE_MAX_LEN	512

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

#define HOOK_IOCTL_NUM 	'm'
#define HOOK_INIT	_IO(HOOK_IOCTL_NUM, 0)
#define HOOK_STOP	_IO(HOOK_IOCTL_NUM, 1)
#define HOOK_ADD	_IOW(HOOK_IOCTL_NUM, 2, struct finder_info)

struct err_type err_info;

unsigned long get_symbol_addr(char *name)
{
	FILE *fd;
	char sname[LINE_MAX_LEN];
	char line[LINE_MAX_LEN];
	char *token;
	int pos;
	bool found = false;
	unsigned long addr = 0;
	int ret = 0;

	fd = fopen("/proc/kallsyms", "r");
	if (!fd) {
		return 0;
	}

	while (fgets(line, LINE_MAX_LEN , fd)) {
		token = strtok(line, " ");
		pos = 0;
		while (token != NULL) {
			switch(pos) {
			case 0:
				sscanf(token, "%p", (void **) &addr);
				break;
			case 2:
				sscanf(token, "%s", sname);
				break;
			default:
				break;
			}
			pos += 1;
			token = strtok(NULL, " ");
		}
		if (!strcmp(name, sname)) {
			found = true;
			break;
		}
	}

	if (found) {
		ret = addr;
	}
	fclose(fd);
	return ret;
}

int add_hook(char *symbol, int fd)
{
	struct finder_info finfo = {0};
	int err = 0;

	finfo.func.name = symbol;
	finfo.func.len = strlen(symbol);

	finfo.addr = get_symbol_addr(symbol);
	if (!finfo.addr) {
		_set_err("Symbol %s not found", symbol);
	}

	printf("+ Add hook\n");
	if (ioctl(fd, HOOK_ADD, (void *) &finfo) < 0) {
		printf("err %d\n", __LINE__);
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
		if (ioctl(fd, HOOK_STOP) < 0) {
			_exit_err_free("Hook removal error: %s", strerror(errno));
		}
		goto free;
	}

	if (!symbol_set) {
		if (sprintf(symbol, "%s", "load_msg") < 0) {
			_exit_err_free("Sprintf error");
		}
	}

	if (add_hook(symbol, fd)) {
		_exit_err_free(err_info.desc);
	}
	printf("+ Install hook\n");
	if (ioctl(fd, HOOK_INIT) < 0) {
		_exit_err_free("Error installing hook: %s", strerror(errno));
	}

free:
	if (fd)
		if (close(fd))
			_exit_err("Error closing device: %s", strerror(errno));
out:
	if (err)
		fprintf(stderr, "%s\n", err_msg);

	return err;
}
