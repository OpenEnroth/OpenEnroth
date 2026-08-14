load("@rules_cc//cc:cc_library.bzl", "cc_library")

# backward-cpp — stack trace library. Not used on Android.
cc_library(
    name = "backward_cpp",
    srcs = ["backward.cpp"],
    hdrs = ["backward.hpp"],
    includes = ["."],
    # On Linux, libdwarf gives stack traces file/line info. Without the define
    # backward.hpp silently falls back to backtrace_symbol (function names
    # only) and the dwarf link inputs are dead weight. Propagated via defines:
    # backward.hpp branches on it, so every includer must agree with
    # backward.cpp - dependents also need the matching -I, see
    # //src/Library/StackTrace. Dwarf stays off in the flatpak sandbox: its
    # libdwarf v0.11+ headers dropped the old API backward.hpp uses
    # (dwarf_elf_init), and cmake's BackwardConfig.cmake never found them
    # there either.
    copts = select({
        "@//bazel:linux_flatpak": [],
        "@platforms//os:linux": ["-I/usr/include/libdwarf"],
        "//conditions:default": [],
    }),
    defines = select({
        "@//bazel:linux_flatpak": [],
        "@platforms//os:linux": ["BACKWARD_HAS_DWARF=1"],
        "//conditions:default": [],
    }),
    linkopts = select({
        "@platforms//os:linux": [
            "-ldwarf",
            "-lelf",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
