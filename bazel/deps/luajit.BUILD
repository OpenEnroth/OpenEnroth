# LuaJIT — two-stage cross-compilation build using rules_foreign_cc make().
#
# Stage 1: Build `buildvm` for the HOST architecture.
# Stage 2: Use `buildvm` to generate minilua, lj_bcdef.h, lj_ffdef.h, etc.
#          for the TARGET architecture.
#
# Android NDK 28 uses LLVM/Clang exclusively. The HOST_CC is the host-side
# compiler (for buildvm); CROSS + TARGET_FLAGS drive the target compilation.
#
# Note: select() keys in injected BUILD files must use @platforms// labels,
# not //bazel/platforms labels (which live in the main repo, not here).

load("@rules_foreign_cc//foreign_cc:defs.bzl", "make")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

make(
    name = "luajit",
    lib_source = ":all_srcs",
    out_static_libs = ["libluajit-5.1.a"],
    # LuaJIT installs headers to include/luajit-2.1/; expose that subdirectory
    # so #include <lua.h> works (matching CMake's include_directories behaviour).
    out_include_dir = "include/luajit-2.1",
    env = select({
        # All Android targets share the same HOST_CC; CROSS is set
        # per-architecture by the Android NDK toolchain wrapper.
        "@platforms//os:android": {
            "HOST_CC": "gcc",
        },
        "//conditions:default": {},
    }),
    visibility = ["//visibility:public"],
)
