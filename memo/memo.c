// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "memo.h"

#define DEVICE			"tracer"
#define DEBUGFS			"/sys/kernel/debug"
#define LINE_MAX_LEN		512
#define OPT_STR_MAX_LEN		64

struct fname {
	char *name;
	int len;
};

struct finder_info {
	unsigned long addr;
	struct fname func;
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
struct memo_args {
	char option;
	char *long_opt;
	int has_arg;
	char *desc;
};

static struct memo_args args[] = {
	{'s', "symbol", required_argument, "Symbol to hook"},
	{'r', "stop",   no_argument,       "Stop the tracer"},
	{'i', "init",   no_argument,       "Initiate the tracer"},
	{'h', "help",   no_argument,       "Display this help and exit"}
};

void populate_long_opts(struct option *long_options) {
	for (int i=0; i<sizeof(args)/sizeof(struct memo_args); i++) {
		long_options[i].name = args[i].long_opt;
		long_options[i].has_arg = args[i].has_arg;
		long_options[i].flag = 0;
		long_options[i].val = args[i].option;
	}
}

void get_opts_string(char *opt_str) {
	char opt[2];

	for (int i=0; i<sizeof(args)/sizeof(struct memo_args); i++) {
		sprintf(opt, "%c", args[i].option);
		strcat(opt_str, opt);
		if (args[i].has_arg)
			strcat(opt_str, ":");
	}
}

void help_menu()
{
	printf("Usage: memo [OPTION]...\n");
	printf("Instrument the FindingMemo hooking framework.\n\n");

	printf("Arguments:\n");
	for (int i=0; i<sizeof(args)/sizeof(struct memo_args); i++) {
		if (strlen(args[i].long_opt))
			printf("  -%c, --%s\t\t%s\n", args[i].option,
			       args[i].long_opt, args[i].desc);
		else
			printf("  -%c\t\t\t%s\n", args[i].option, args[i].desc);
	}
	printf("\n");
}


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
	printf("Hook added for %s.\n", symbol);
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
	bool hook_init = false;
	bool hook_stop = false;
	int option_index;
	struct option long_options[sizeof(args)];
	char opt_str[OPT_STR_MAX_LEN];

	populate_long_opts(long_options);
	get_opts_string(opt_str);
	while (1) {
		opt = getopt_long(argc, argv, opt_str,
		                  long_options, &option_index);
		if (opt == -1)
			break;

		switch (opt) {
			case 's':
				strncpy(symbol, optarg, sizeof(symbol));
				symbol_set = true;
				break;
			case 'r':
				hook_stop = true;
				break;
			case 'i':
				hook_init = true;
				break;
			case 'h':
				help_menu();
				return 0;
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

	if (hook_stop) {
		if (ioctl(fd, HOOK_STOP) < 0) {
			_exit_err_free("Hook removal error: %s", strerror(errno));
		}
		printf("Linux hooking stopped.\n");
		goto free;
	}

	if (hook_init) {
		if (ioctl(fd, HOOK_INIT) < 0) {
			_exit_err_free("Error installing hook: %s", strerror(errno));
		}
		printf("Linux hooking initiated.\n");
		goto free;
	}

	if (!symbol_set) {
		fprintf(stderr, "%s", "Symbol missing\n");
		goto free;
	}

	if (add_hook(symbol, fd)) {
		_exit_err_free(err_info.desc);
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
