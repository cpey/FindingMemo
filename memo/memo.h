// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef TEST_H_
#define TEST_H_

#define ERR_MAX_LEN 256
#define SYM_MAX_LEN 256

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

