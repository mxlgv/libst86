/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 Maxim Logaev
 */

/* Test of the st86.h using glibc backtrace functions. */

#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <execinfo.h>

#include "st86.h"

/* Always enable assertion. */
#undef NDEBUG

/*
 * Request backtrace depth.
 * Placed as a global variable so as not to pass it through multiple calls.
 */
size_t depth_requested = 0;

void test_main(void)
{
    uintptr_t *const addrs_expected =
        malloc(depth_requested * sizeof(*addrs_expected));
    assert(addrs_expected);

    uintptr_t *const addrs = malloc(depth_requested * sizeof(*addrs));
    assert(addrs);

    const int depth_expected =
        backtrace((void **)addrs_expected, depth_requested);
    assert(depth_expected);

    char **const strs_expected =
        backtrace_symbols((void **)addrs_expected, depth_expected);
    assert(strs_expected);

    char *tmpbuf_expected;
    size_t dummy_size;
    FILE *const memfile_expected =
        open_memstream(&tmpbuf_expected, &dummy_size);
    assert(memfile_expected);

    for (int i = 0; i < depth_expected; i++) {
        fputs(strs_expected[i], memfile_expected);
        fputc('\n', memfile_expected);
    }
    fputc('\0', memfile_expected);
    fflush(memfile_expected);

    char *tmpbuf;
    FILE *const memfile = open_memstream(&tmpbuf, &dummy_size);
    assert(memfile);

    st86_stacktrace(depth_requested, memfile);
    fputc('\0', memfile);
    fflush(memfile);

    // The return addresses for backtrace() and st86_stacktrace() are different.
    // Skip first line:
    tmpbuf_expected = strchr(tmpbuf_expected, '\n');
    assert(tmpbuf_expected);
    tmpbuf = strchr(tmpbuf, '\n');
    assert(tmpbuf);

    printf("Expected:%s", tmpbuf_expected);
    printf("Actual:%s", tmpbuf);
    assert(!strcmp(tmpbuf, tmpbuf_expected));

    fclose(memfile_expected);
    fclose(memfile);

    free(strs_expected);

    free(tmpbuf_expected);
    free(tmpbuf);

    free(addrs_expected);
    free(addrs);
}

// Test call tree:
void dummy_func6(void) { test_main(); }
void dummy_func5(void) { dummy_func6(); }
__attribute__((visibility("hidden"))) void dummy_func4(void) { dummy_func5(); }
void dummy_func3(void) { dummy_func4(); }
void dummy_func2(void) { dummy_func3(); }
void dummy_func1(void) { dummy_func2(); }

int main(void)
{
    setbuf(stdout, NULL);

    puts("--- Test get current frame ----");
    printf("frame_addr: %p == %p\n", (void *)st86_get_frame_addr(),
           __builtin_frame_address(0));
    assert(st86_get_frame_addr() == __builtin_frame_address(0));
    puts("OK");

    puts("------ Test normal depth ------");
    depth_requested = 5;
    dummy_func1();
    puts("OK");

    puts("--- Test overflowing depth ----");
    depth_requested = 10000;
    dummy_func1();
    puts("OK");

    return 0;
}
