// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 Carles Pey <cpey@pm.me>
 */

#ifndef LIBFM_H_
#define LIBFM_H_

enum ERR_TYPE {
	SYMNOTFOUND = 1,
	HOOKADDFAIL = 2,
};

int fm_add_hook(char *);
int fm_init();
int fm_stop();

#endif /* LIBFM_H_ */
