// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#include "hook.h"
#include <linux/oom.h>

struct msg_msg *msg;

FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src,
		size_t, len)
{
	atomic_set(&curr_hook->mutex, false);
	msg = FM_HOOK_FUNC_PTR(load_msg)(src, len);
	atomic_set(&curr_hook->mutex, true);
	pr_info("fmemo: load_msg(): msg addr: %px\n", msg);
	return msg;
}

FM_HOOK_ATTR_DEFINE(load_msg)
{
	return snprintf(buf, PAGE_SIZE, "%px\n", msg);
}

FM_HOOK_FUNC_DEFINE1(free_msg, void, struct msg_msg *, msg)
{
	atomic_set(&curr_hook->mutex, false);
	FM_HOOK_FUNC_PTR(free_msg)(msg);
	atomic_set(&curr_hook->mutex, true);
	pr_info("fmemo: free_msg(): msg addr: %px\n", msg);
}

FM_HOOK_ATTR_DEFINE(free_msg)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", 26);
}

FM_HOOK_FUNC_DEFINE3(sockfd_lookup_light, struct socket *, int, fd, int *, err,
		int *, fput_needed)
{
	struct socket *sock;
	atomic_set(&curr_hook->mutex, false);
	sock = FM_HOOK_FUNC_PTR(sockfd_lookup_light)(fd, err, fput_needed);
	atomic_set(&curr_hook->mutex, true);
	pr_info("fmemo: sockfd_lookup_light(): sock addr: %px\n", sock);
	return sock;
}

/*
 * Disable OOM killer
 */
FM_HOOK_FUNC_DEFINE2(oom_kill_process, void, struct oom_control *, oc,
		const char *, message)
{
}

FM_HOOK_FUNC_DEFINE1(out_of_memory, bool, struct oom_control *, oc)
{
	return false;
}
