# libunwind

The `unw_*` API from LLVM's libunwind, and nothing else.

Taken from https://github.com/llvm/llvm-project, branch `release/19.x`, commit `cd708029e0b2869e80abe31ddb175f7c35361f90`,
directory `libunwind/`. Only `libunwind.cpp`, the two register save/restore assembly files, and the headers they pull
in are here. `UnwindLevel1*.c` and friends are deliberately left out: they are the `_Unwind_*` runtime that C++
exceptions unwind through, and linking a second copy of that next to libgcc's is how exception handling breaks. The
subset here defines no `_Unwind_*` symbol at all, so exceptions keep going through the system runtime.

What this buys is `unw_init_local` on a context we supply, which is how a crash handler walks the stack the crash
happened on rather than its own. The libgcc unwinder has no such entry point.
