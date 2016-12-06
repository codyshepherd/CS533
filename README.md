# threads

A user-level threads library written in C and x86\_64 ABI.

M:N Phase 0

## Compilation statement

Compilation statements may depend on which test program you want to run.

All test programs live in `scheduled/tests/`, and must be moved into `scheduled/` for compilation.

`cd scheduled`

`gcc *.c *.s -lrt -I`, plus any of your favorite warning or debugger flags.

## Notes

This program is dependent on [`libatomic_ops`](https://github.com/ivmai/libatomic_ops) for atomic locking instructions. 

This program will only compile correctly on Linux with `gcc`. Note that `clang` is not
a working substitute. 
