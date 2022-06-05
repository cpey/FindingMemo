// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Carles Pey <cpey@pm.me>
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "memo.h"
#include "libfm.h"

#define OPT_STR_MAX_LEN		64

struct err_type err_info;
struct memo_args {
	char option;
	char *long_opt;
	int has_arg;
	char *desc;
};

static struct memo_args args[] = {
	{'a', "add",    required_argument, "Add hook symbol"},
	{'s', "stop",   no_argument,       "Stop kernel instrumentation"},
	{'i', "init",   no_argument,       "Initiate kernel instrumentation"},
	{'h', "help",   no_argument,       "Display this help and exit"}
};

static void populate_long_opts(struct option *long_options) {
	for (int i=0; i<sizeof(args)/sizeof(struct memo_args); i++) {
		long_options[i].name = args[i].long_opt;
		long_options[i].has_arg = args[i].has_arg;
		long_options[i].flag = 0;
		long_options[i].val = args[i].option;
	}
}

static void get_opts_string(char *opt_str) {
	char opt[2];

	for (int i=0; i<sizeof(args)/sizeof(struct memo_args); i++) {
		sprintf(opt, "%c", args[i].option);
		strcat(opt_str, opt);
		if (args[i].has_arg)
			strcat(opt_str, ":");
	}
}

static void help_menu()
{
	printf("Usage: memo [OPTION]...\n");
	printf("Configuration client to the FindingMemo hooking framework.\n\n");

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

int main(int argc, char** argv)
{
	char device[50];
	char err_msg[ERR_MAX_LEN];
	char symbol[SYM_MAX_LEN];
	int err = 0, opt;
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
			case 'a':
				strncpy(symbol, optarg, sizeof(symbol));
				symbol_set = true;
				break;
			case 's':
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

	if (hook_stop) {
		if (fm_stop() > 0) {
			_exit_err("Hook removal error: %s", strerror(errno));
		}
		printf("Linux hooking stopped.\n");
		goto out;
	}

	if (hook_init) {
		if (fm_init() > 0) {
			_exit_err("Error installing hook: %s", strerror(errno));
		}
		printf("Linux hooking initiated.\n");
		goto out;
	}

	if (!symbol_set) {
		fprintf(stderr, "%s", "Symbol missing\n");
		goto out;
	}

	if ((err = fm_add_hook(symbol)) > 0) {
		switch (err) {
			case  SYMNOTFOUND:
				_exit_err("Symbol %s not found", symbol);
				break;
			case  HOOKADDFAIL:
				_exit_err("Error adding hook: %s", strerror(errno));
				break;
		}
	}
	printf("Hook added for %s.\n", symbol);

out:
	if (err)
		fprintf(stderr, "%s\n", err_msg);

	return err;
}
