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

# Per-ABI config_settings for Android cross-compilation args below.
# Note: //bazel/platforms:* labels can't be used in injected BUILD files (they live
# in the main repo), so we define local config_settings using @platforms// labels.
config_setting(
    name = "_android_arm64",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:arm64"],
)

config_setting(
    name = "_android_armv7",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:armv7"],
)

config_setting(
    name = "_android_x86_64",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:x86_64"],
)

# linux_x86: identified by --define=oe_build_arch=x86_32 (set in .bazelrc for
# --config=linux_x86). Can't use constraint_values/@platforms//cpu:x86_32 because
# --config=linux_x86 uses the host platform (x86_64) with -m32 flags, not a 32-bit
# target platform.
config_setting(
    name = "_linux_x86",
    define_values = {"oe_build_arch": "x86_32"},
)

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
    # Two select()s are concatenated (Bazel 7+ supports list select() addition).
    # First: OS-level flags. Second: per-ABI Android TARGET_FLAGS.
    args = select({
        # Android: HOST tools (buildvm, minilua) must run on the Linux build host.
        # CFLAGS/LDFLAGS contain Android NDK-specific flags (--target=aarch64-... etc.).
        # Clear CFLAGS/LDFLAGS so NDK flags don't leak into HOST tool compilation.
        #
        # "-e": LuaJIT's Makefile has "CC = $(DEFAULT_CC)" (defaults to "gcc"),
        # which overrides the CC environment variable that rules_foreign_cc sets to NDK
        # clang. The -e flag (--environment-overrides) tells make to let environment
        # variables take precedence over Makefile variable assignments. Since
        # rules_foreign_cc exports CC=/path/to/ndk/clang in the build environment
        # BEFORE invoking make, -e ensures LuaJIT uses NDK clang instead of gcc.
        # Command-line variables (CFLAGS=, LDFLAGS=, TARGET_FLAGS=...) still have
        # highest precedence and override both env and Makefile (make semantics).
        # Without --target, NDK clang targets the x86_64 host, so HOST_CC = $(CC)
        # also compiles HOST tools correctly. The NDK target triple is supplied
        # per-ABI via TARGET_FLAGS below.
        # TARGET_LJARCH is set per-ABI to bypass LuaJIT's auto-detect (which runs CC
        # without --target and would misdetect the host arch when CFLAGS is empty).
        "@platforms//os:android": [
            "-e",
            "CFLAGS=",
            "LDFLAGS=",
            # LuaJIT also builds a 'luajit' executable for the TARGET (ARM64 Android).
            # Host 'strip' can't handle ARM64 ELF: "Unable to recognise the format".
            # We only need libluajit-5.1.a, not the executable, so make strip a no-op.
            "TARGET_STRIP=true",
        ],
        # Linux: HOST_CC=gcc-14 (bare, no Bazel toolchain wrapper) ensures HOST
        # tools are compiled by the host compiler. On linux_x86, Bazel injects -m32
        # via cc_wrapper, making HOST tools 32-bit. They still run on x86_64 since
        # gcc-14-multilib provides 32-bit runtime support.
        # LDFLAGS= clears Bazel-injected linker flags for the luajit executable link
        # step. On linux_x86, those flags include -L paths to gcc-13 multilib dirs
        # that don't have 32-bit libstdc++, causing link failure.
        # linux_x86 additionally sets LDFLAGS=-m32 in the second select() below to
        # restore the -m32 flag needed for 32-bit linking (GNU make uses the last
        # command-line assignment, so it overrides this empty LDFLAGS= value).
        "@platforms//os:linux": [
            "HOST_CC=gcc-14",
            "HOST_CFLAGS=-O2",
            "HOST_LDFLAGS=",
            "LDFLAGS=",
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
    }) + select({
        # Per-ABI Android args: TARGET_LJARCH bypasses auto-detection (NDK raw clang
        # without --target would detect host arch), TARGET_FLAGS supplies the NDK
        # --target triple so CC (NDK clang) targets the correct Android ABI.
        # API 31 matches the CI NDK 28.1.13356709 configuration.
        # BUILDMODE=static: LuaJIT's default (mixed) also builds libluajit.so which
        # requires Android linker flags we cleared with LDFLAGS=. Static-only avoids
        # the DYNLINK step entirely.
        # Note: TARGET_FLAGS must NOT contain spaces — rules_foreign_cc passes each
        # args[] element as one shell word, and make splits on spaces, treating anything
        # after the first space in "VAR=val flags" as a separate make target.
        ":_android_arm64": [
            "TARGET_LJARCH=arm64",
            "TARGET_SYS=Linux",
            "TARGET_FLAGS=--target=aarch64-linux-android31",
            "BUILDMODE=static",
        ],
        ":_android_armv7": [
            "TARGET_LJARCH=arm",
            "TARGET_SYS=Linux",
            # LuaJIT sets TARGET_ARCH=-march=armv7-a for arm, which ends up in
            # HOST_ACFLAGS and breaks x86_64 gcc. Clear it; the NDK clang triple
            # (--target=armv7a-linux-androideabi31) implies the right march. LuaJIT
            # also adds -mfpu=vfpv3-d16 to TARGET_XCFLAGS automatically for arm.
            "TARGET_ARCH=",
            "TARGET_FLAGS=--target=armv7a-linux-androideabi31",
            "BUILDMODE=static",
            # DUALNUM is handled via patch_cmds in MODULE.bazel, which patches
            # dynasm/dynasm.lua to auto-define DUALNUM when processing vm_arm.dasc.
            # This bypasses the Make variable system (TARGET_TESTARCH detection fails
            # in Bazel's cross-compilation sandbox). No command-line DASM_AFLAGS needed.
        ],
        ":_android_x86_64": [
            "TARGET_LJARCH=x64",
            "TARGET_SYS=Linux",
            "TARGET_FLAGS=--target=x86_64-linux-android31",
            "BUILDMODE=static",
        ],
        # linux_x86: LuaJIT default (mixed) builds libluajit.so which tries to link
        # against 32-bit libstdc++.so — not always available. Use static-only to skip
        # the shared lib link step entirely.
        # LDFLAGS=-m32 overrides the LDFLAGS= from the linux OS section above (GNU
        # make uses the last command-line assignment). This restores the -m32 flag so
        # both HOST (minilua.o compiled 32-bit via CCOPTIONS) and TARGET link steps
        # produce 32-bit output, avoiding the i386/x86-64 arch mismatch error.
        ":_linux_x86": ["BUILDMODE=static", "LDFLAGS=-m32"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
