# libst86
__libst86__ is the simplest stacktrace library for __x86__ and __x86_64__ that uses symbols for the dynamic linker to display function names.

In fact, this library was written to study the basic understanding of how a stack trace works and does not provide any particularly useful functionality.

## Compatibility
- OS: GNU/Linux;
- Arch: x86, x86_64.

## Building
Run:
```sh
ninja
```
For non-__GCC__ compilation you can change the `c`, `cflags` and `nasm_obj_format` variables in the [build.ninja](./build.ninja) file.

## Running test
Run:
```sh
ninja tests
```

## Example
Please see [exampe.c](./example.c).
