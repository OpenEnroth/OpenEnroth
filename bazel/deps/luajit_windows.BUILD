# LuaJIT Windows prebuilt wrapper.
#
# This repository rule maps to the workspace root so it can access both:
#   thirdparty/luajit/src/*.h  — source headers from the submodule
#   build/thirdparty/Debug/libluajit.lib — CMake-built static library
#   build/thirdparty/luajit.h  — CMake-generated header
#
# Requires a prior CMake build (`cmake -B build -S .`) to produce
# libluajit.lib and luajit.h before running Bazel on Windows.

cc_import(
    name = "_luajit_lib",
    static_library = "build/thirdparty/Debug/libluajit.lib",
    hdrs = glob(
        ["thirdparty/luajit/src/*.h", "build/thirdparty/luajit.h"],
        allow_empty = True,
    ),
)

cc_library(
    name = "luajit_prebuilt",
    includes = ["thirdparty/luajit/src", "build/thirdparty"],
    deps = [":_luajit_lib"],
    visibility = ["//visibility:public"],
)
