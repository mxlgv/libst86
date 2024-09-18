/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 Maxim Logaev
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <dlfcn.h>

#include "st86.h"

typedef struct stack_frame_s {
    struct stack_frame_s *next_frame;
    uintptr_t ret_addr;
} stack_frame_t;

static uintptr_t get_stack_bottom(void)
{
    uintptr_t stack_bottom_addr = 0;
    uintptr_t dummy;

    char line[100];
    FILE *const maps_file = fopen("/proc/self/maps", "r");
    if (!maps_file) {
        return stack_bottom_addr;
    }

    while (fgets(line, sizeof(line), maps_file)) {
        if (strstr(line, "[stack]")) {
            sscanf(line, "%" SCNxPTR "-%" SCNxPTR, &dummy, &stack_bottom_addr);
            break;
        }
    }

    fclose(maps_file);
    return stack_bottom_addr;
}

static stack_frame_t *get_next_frame(const stack_frame_t *stack_frame,
                                     uintptr_t stack_bottom_addr)
{
    if ((uintptr_t)stack_frame->next_frame < (uintptr_t)stack_frame ||
        (uintptr_t)stack_frame->next_frame > stack_bottom_addr) {
        return NULL;
    }

    return stack_frame->next_frame;
}

void st86_stacktrace(size_t depth, FILE *stream)
{
    const stack_frame_t *stack_frame = st86_get_frame_addr();
    const uintptr_t stack_bottom_addr = get_stack_bottom();

    for (size_t frame_count = 0; stack_frame && frame_count < depth;
         frame_count++) {
        Dl_info info;
        if (dladdr((void *)stack_frame->ret_addr, &info)) {
            if (info.dli_fname) {
                fputs(info.dli_fname, stream);
            }

            fputc('(', stream);

            if (info.dli_sname) {
                fputs(info.dli_sname, stream);
            }

            char sign = '+';
            uintptr_t ret_addr_diff;
            if (info.dli_saddr) {
                if (stack_frame->ret_addr >= (uintptr_t)info.dli_saddr) {
                    ret_addr_diff =
                        stack_frame->ret_addr - (uintptr_t)info.dli_saddr;
                } else {
                    ret_addr_diff =
                        (uintptr_t)info.dli_saddr - stack_frame->ret_addr;
                    sign = '-';
                }
            } else {
                ret_addr_diff =
                    stack_frame->ret_addr - (uintptr_t)info.dli_fbase;
            }

            fprintf(stream, "%c0x%" PRIxPTR ") ", sign, ret_addr_diff);
        }

        fprintf(stream, "[0x%" PRIxPTR "]\n", stack_frame->ret_addr);
        stack_frame = get_next_frame(stack_frame, stack_bottom_addr);
    }
}
