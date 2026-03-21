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
    # HOST_CC must be passed as a make command-line variable (not env) because
    # LuaJIT's src/Makefile unconditionally assigns `HOST_CC= $(CC)`, which
    # overrides environment variables. Command-line make variables have the
    # highest precedence and do override Makefile assignments.
    #
    # HOST_CFLAGS and HOST_LDFLAGS are used in:
    #   HOST_ACFLAGS = $(CCOPTIONS) $(HOST_XCFLAGS) $(TARGET_ARCH) $(HOST_CFLAGS)
    #   HOST_ALDFLAGS = $(LDOPTIONS) $(HOST_XLDFLAGS) $(HOST_LDFLAGS)
    # Note: CCOPTIONS includes $(CFLAGS) (Bazel-injected) which may contain -m32
    # or arch-specific flags. HOST_CFLAGS=-O2 adds optimization; HOST_LDFLAGS=
    # prevents injected LDFLAGS from conflicting with HOST tool link.
    args = select({
        # Android: HOST tools (buildvm, minilua) must run on the Linux build host.
        # Use bare gcc as HOST_CC so host tools are compiled for the build machine.
        "@platforms//os:android": [
            "HOST_CC=gcc",
        ],
        # Linux: HOST_CC=gcc-14 (bare, no Bazel toolchain wrapper) ensures HOST
        # tools are compiled by the host compiler. On linux_x86, Bazel injects -m32
        # via cc_wrapper, making HOST tools 32-bit. They still run on x86_64 since
        # gcc-14-multilib provides 32-bit runtime support.
        "@platforms//os:linux": [
            "HOST_CC=gcc-14",
            "HOST_CFLAGS=-O2",
            "HOST_LDFLAGS=",
        ],
        # macOS: Use bare 'cc' (system clang) so HOST tools are compiled without
        # arch-specific flags from Bazel's cc_wrapper. darwin_x86_64 builds run on
        # arm64 runners (macos-15), so HOST tools must be arm64, not x86_64.
        # Note: preinstalled_make_toolchain is also enabled (see .bazelrc) to avoid
        # the bundled rules_foreign_cc make binary segfaulting on LuaJIT's Makefile.
        "@platforms//os:macos": [
            "HOST_CC=cc",
            "HOST_CFLAGS=-O2",
            "HOST_LDFLAGS=",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
