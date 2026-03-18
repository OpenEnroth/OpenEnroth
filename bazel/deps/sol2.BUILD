cc_library(
    name = "sol2",
    hdrs = glob(["include/sol/**/*.hpp", "include/sol/**/*.h"]),
    includes = ["include"],
    # SOL_EXCEPTIONS_ALWAYS_UNSAFE=1 prevents sol2 from propagating C++ exceptions
    # into LuaJIT, which does not support C++ exceptions crossing its boundary.
    defines = ["SOL_EXCEPTIONS_ALWAYS_UNSAFE=1"],
    deps = ["@@//bazel/system:luajit"],
    visibility = ["//visibility:public"],
)
