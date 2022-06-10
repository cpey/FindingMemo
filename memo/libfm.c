// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Carles Pey <cpey@pm.me>
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "memo.h"
#include "libfm.h"

static unsigned long get_symbol_addr(char *name, unsigned long *addr)
{
	FILE *fd;
	char sname[LINE_MAX_LEN];
	char line[LINE_MAX_LEN];
	char *token;
	int pos;
	bool found = false;
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
				sscanf(token, "%llx", (void *) addr);
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

	fclose(fd);
	return found;
}

static int add_hook(char *symbol, int fd)
{
	struct finder_info finfo = {0};
	int found = 0;

	finfo.func.name = symbol;
	finfo.func.len = strlen(symbol);
	found = get_symbol_addr(symbol, finfo.addr);
	if (!found) {
		return SYMNOTFOUND;
	}
	if (ioctl(fd, HOOK_ADD, (void *) &finfo) < 0) {
		return HOOKADDFAIL;
	}

	return 0;
}

static int open_device() 
{
	int fd, err;
	char device[50];

	if ((err = sprintf(device, "%s/%s", DEBUGFS, DEVICE)) < 0) {
		return -err;
	}

	if ((fd = open(device, O_WRONLY)) < 0 ) {
		return -fd;
	}

	return fd;
}

int fm_add_hook(char *symbol)
{
	int fd, err = 0;

	if ((fd = open_device()) < 0) {;
		return -fd;
	}

	err = add_hook(symbol, fd);
	close(fd);
	return err;
}

int fm_init()
{
	int fd, err = 0;

	if ((fd = open_device()) < 0) {;
		return -fd;
	}

	if ((err = ioctl(fd, HOOK_INIT)) < 0) {
		err = -err;
	}
	close(fd);
	return err;
}

int fm_stop()
{
	int fd, err = 0;

	if ((fd = open_device()) < 0) {;
		return -fd;
	}

	if ((err = ioctl(fd, HOOK_STOP)) < 0) {
		err = -err;
	}

	close(fd);
	return err;
}
