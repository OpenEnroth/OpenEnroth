"""Debug-mode standard library assertions, cmake's
add_compile_definitions($<$<CONFIG:Debug>:...>) block.

`defines` propagate to everything that depends on the target carrying them, so
these ride the dependency graph: putting them on //src/Utility:utility and the
handful of first-party libraries that don't depend on it covers all of src and
test without touching external deps. MSVC needs no entry - the debug CRT that
-c dbg selects already enables checked iterators.
"""

STDLIB_HARDENING_DEFINES = select({
    # Enable assertions in libstdc++.
    "//bazel:linux_dbg": ["_GLIBCXX_ASSERTIONS"],
    # Enable assertions in libc++.
    "//bazel:macos_dbg": ["_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG"],
    "//bazel:android_dbg": ["_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG"],
    "//conditions:default": [],
})
