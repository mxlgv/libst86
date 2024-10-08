/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 Maxim Logaev
 */

#ifndef _ST86_H_
#define _ST86_H_

/* So that you can use the dladdr() from "-ldl". */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Print everything to the end of the stack. */
#define ST86_DEPTH_ALL -1

/*
 * Structure describing the part of the x86/x86_64
 * stack frame for unwinding.
 */
typedef struct stack_frame_s {
    struct stack_frame_s *next_frame;  // saved ebp (x86) or rbp (x86_64)
    uintptr_t ret_addr;                // saved eip (x86) or rip (x86_64)
} stack_frame_t;

/*
 * An assembly inline written in such a way as to avoid
 * generating a prologue and epilogue.
 * According to x86 CDECL, the return value is placed in eax or rax
 * respectively:
 */
__asm__(
    ".global st86_get_frame_addr\n\t"
    "st86_get_frame_addr:\n"
#ifdef __i386__
    "mov %ebp, %eax\n\t"  //
#else
    "mov %rbp, %rax\n\t"
#endif
    "ret");

extern stack_frame_t *st86_get_frame_addr(void);

/*
 * Parse "/proc/self/maps" to get the bottom of the stack
 * for the current process.
 */
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

/* Get the address of the next frame and check if it is valid. */
static stack_frame_t *get_next_frame(const stack_frame_t *stack_frame,
                                     uintptr_t stack_bottom_addr)
{
    if ((uintptr_t)stack_frame->next_frame < (uintptr_t)stack_frame ||
        (uintptr_t)stack_frame->next_frame > stack_bottom_addr) {
        return NULL;
    }

    return stack_frame->next_frame;
}

/* General function. Prints to stream stack trace. */
void st86_stacktrace(size_t depth, FILE *stream)
{
    const stack_frame_t *stack_frame = st86_get_frame_addr();
    const uintptr_t stack_bottom_addr = get_stack_bottom();

    for (size_t frame_count = 0; stack_frame && frame_count < depth;
         frame_count++) {
        Dl_info info;

        // dladdr() allows you to find out from the linker speaker whether
        // the address belongs to any symbol.
        if (dladdr((void *)stack_frame->ret_addr, &info)) {
            if (info.dli_fname) {
                fputs(info.dli_fname, stream);
            }

            fputc('(', stream);

            if (info.dli_sname) {
                fputs(info.dli_sname, stream);
            }

            // If the symbol is not found,
            // calculate the offset from the file base.
            const uintptr_t ret_addr_offset =
                info.dli_saddr
                    ? stack_frame->ret_addr - (uintptr_t)info.dli_saddr
                    : stack_frame->ret_addr - (uintptr_t)info.dli_fbase;

            fprintf(stream, "+0x%" PRIxPTR ") ", ret_addr_offset);
        }

        fprintf(stream, "[0x%" PRIxPTR "]\n", stack_frame->ret_addr);

        // If the address is not valid, it returns NULL.
        stack_frame = get_next_frame(stack_frame, stack_bottom_addr);
    }
}

#ifdef __cplusplus
}
#endif

#endif  // _ST86_H_
