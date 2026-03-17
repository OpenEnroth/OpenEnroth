# LuaJIT — two-stage cross-compilation build using rules_foreign_cc make().
#
# Stage 1: Build `buildvm` for the HOST architecture.
# Stage 2: Use `buildvm` to generate minilua, lj_bcdef.h, lj_ffdef.h, etc.
#          for the TARGET architecture.
#
# This is the most complex dep in the migration. The make() rule from
# rules_foreign_cc handles the environment setup needed for cross-compilation.
#
# References:
#   - luajit-cmake submodule (zhaozg/luajit-cmake) for flag guidance
#   - rules_foreign_cc examples for LuaJIT cross-compilation

load("@rules_foreign_cc//foreign_cc:defs.bzl", "make")

make(
    name = "luajit",
    lib_source = ":",
    out_static_libs = ["libluajit.a"],
    # Platform-specific make flags
    build_data = select({
        "@platforms//os:windows": [],
        "//conditions:default": [],
    }),
    env = select({
        "@platforms//os:android": {
            "HOST_CC": "gcc",
            "CROSS": "aarch64-linux-android-",
        },
        "//conditions:default": {},
    }),
    visibility = ["//visibility:public"],
)
