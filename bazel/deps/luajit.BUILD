# LuaJIT — two-stage cross-compilation build using rules_foreign_cc make().
#
# Stage 1: Build `buildvm` for the HOST architecture.
# Stage 2: Use `buildvm` to generate minilua, lj_bcdef.h, lj_ffdef.h, etc.
#          for the TARGET architecture.
#
# Android NDK 28 uses LLVM/Clang exclusively. The HOST_CC is the host-side
# compiler (for buildvm); CROSS + TARGET_FLAGS drive the target compilation.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "make")

make(
    name = "luajit",
    lib_source = ":",
    out_static_libs = ["libluajit.a"],
    env = select({
        "//bazel/platforms:android_arm64_v8a": {
            "HOST_CC": "gcc",
            "CROSS": "aarch64-linux-android-",
            "TARGET_FLAGS": "-target aarch64-linux-android21",
        },
        "//bazel/platforms:android_armeabi_v7a": {
            "HOST_CC": "gcc",
            "CROSS": "armv7a-linux-androideabi-",
            "TARGET_FLAGS": "-target armv7a-linux-androideabi21 -mfpu=neon",
        },
        "//bazel/platforms:android_x86_64": {
            "HOST_CC": "gcc",
            "CROSS": "x86_64-linux-android-",
            "TARGET_FLAGS": "-target x86_64-linux-android21",
        },
        "//conditions:default": {},
    }),
    visibility = ["//visibility:public"],
)
