// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Carles Pey <cpey@pm.me>
 */

#ifndef HOOK_H_
#define HOOK_H_

int hook_install(FName*);
int hook_remove(FName*);
void hook_init(long);

#endif /* HOOK_H_ */
