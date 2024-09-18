/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 Maxim Logaev
 */

// A small example of using libst86
// to display the entire stacktrace.

#include "st86.h"

void dummy_func6(void) { st86_stacktrace(ST86_DEPTH_ALL, stderr); }
void dummy_func5(void) { dummy_func6(); }
void dummy_func4(void) { dummy_func5(); }
void dummy_func3(void) { dummy_func4(); }
void dummy_func2(void) { dummy_func3(); }
void dummy_func1(void) { dummy_func2(); }

int main(void)
{
    dummy_func1();
    return 0;
}
