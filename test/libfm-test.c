// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Carles Pey <cpey@pm.me>
 */

#include <stdio.h>
#include "libfm.h"

int main()
{
	int ret;

	printf("+ Adding hook\n");
	ret = fm_add_hook("load_msg");
	if (ret) {
		printf("Error adding hook: %d\n", ret);
		goto out;
	}

	printf("+ FM init\n");
	ret = fm_init();
	if (ret) {
		printf("FM init error: %d\n", ret);
		goto out;
	}

	printf("+ FM stop\n");
	ret = fm_stop();
	if (ret) {
		printf("FM stop error: %d\n", ret);
		goto out;
	}

out:
	return ret;
}
