load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "sol2",
    hdrs = glob([
        "include/sol/**/*.hpp",
        "include/sol/**/*.h",
    ]),
    # SOL_EXCEPTIONS_ALWAYS_UNSAFE=1 prevents sol2 from propagating C++ exceptions
    # into LuaJIT, which does not support C++ exceptions crossing its boundary.
    defines = ["SOL_EXCEPTIONS_ALWAYS_UNSAFE=1"],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = ["@openenroth//bazel/system:luajit"],
)
