# LuaJIT built with MSVC via the luajit-cmake wrapper (see luajit_repo.bzl).
# Only used on Windows; other platforms build LuaJIT via make() in luajit.BUILD.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "luajit",
    lib_source = ":all_srcs",
    out_static_libs = ["libluajit.lib"],
    # luajit-cmake installs headers flat into include/luajit/, so #include <lua.h> works,
    # matching the include/luajit-2.1 layout of the make() build.
    out_include_dir = "include/luajit",
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # Use static MSVC runtime (/MT) to match the rest of the build.
        "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
        "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
    },
    visibility = ["//visibility:public"],
)
