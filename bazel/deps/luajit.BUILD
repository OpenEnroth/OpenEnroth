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
        # On Linux, Bazel injects all --copt flags (including -m32 for linux_x86)
        # into CC via its cc_wrapper. LuaJIT's HOST tools (minilua, buildvm) must
        # run on the BUILD HOST (always x86_64), not the target. Setting HOST_CC
        # to bare gcc-14 avoids injecting -m32 into the host tool compilation.
        "@platforms//os:linux": {
            "HOST_CC": "gcc-14",
        },
        # On macOS, Bazel's cc_wrapper injects arch-specific flags. For
        # darwin_x86_64 builds on arm64 runners the default HOSTCC would produce
        # an x86_64 buildvm that segfaults on the arm64 host. Use bare 'cc'
        # (system clang) so HOST tools are compiled for the host CPU (arm64).
        "@platforms//os:macos": {
            "HOST_CC": "cc",
        },
        "//conditions:default": {},
    }),
    visibility = ["//visibility:public"],
)
