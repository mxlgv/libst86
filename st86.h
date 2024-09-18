/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 Maxim Logaev
 */

#ifndef _ST86_H_
#define _ST86_H_

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ST86_DEPTH_ALL -1

void *st86_get_frame_addr(void);
void st86_stacktrace(size_t depth, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif  // _ST86_H_
