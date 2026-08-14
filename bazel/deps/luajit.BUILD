# Native bazel build of LuaJIT mirroring src/Makefile's bootstrap: minilua runs
# dynasm over vm_<arch>.dasc, buildvm (host tool with the target's arch defines)
# emits the lj_*def.h headers and the VM (asm source, or a COFF object for MSVC
# - no assembler needed), then the library compiles lj_*.c + lib_*.c + the VM.
# Dasm flags and defines below are precomputed per (cpu, os) pair from lj_arch.h
# instead of the Makefile's compile-and-grep discovery; all targets are LE.

load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

config_setting(
    name = "_windows",
    constraint_values = ["@platforms//os:windows"],
)

# Strict specialization of _windows, same trick as _linux_x86 below.
config_setting(
    name = "_windows_x86",
    constraint_values = ["@platforms//os:windows"],
    define_values = {"oe_build_arch": "x86_32"},
)

config_setting(
    name = "_macos_arm64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "_macos_x86_64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "_linux_arm64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "_linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

# linux_x86 builds use the host platform (x86_64 constraints) with -m32 flags and
# --define=oe_build_arch=x86_32 (see .bazelrc). The extra constraint_values make
# this a strict specialization of _linux_x86_64 so select() resolution is
# unambiguous when both match.
config_setting(
    name = "_linux_x86",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    define_values = {"oe_build_arch": "x86_32"},
)

config_setting(
    name = "_android_arm64",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "_android_armv7",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:armv7",
    ],
)

config_setting(
    name = "_android_x86_64",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "_android_x86",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_32",
    ],
)

# ─── Build variants ──────────────────────────────────────────────────────────
# One entry per distinct (arch, LUAJIT_OS) combination. Android maps to
# LUAJIT_OS_LINUX, exactly as in src/Makefile.

_VARIANTS = {
    "x64_linux": {
        "dasc": "src/vm_x64.dasc",
        "dasm_flags": "-D ENDIAN_LE -D P64 -D JIT -D FFI -D FPU -D HFABI -D VER=",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_x64",
            "LUAJIT_OS=LUAJIT_OS_LINUX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "elfasm",
    },
    "x64_osx": {
        "dasc": "src/vm_x64.dasc",
        "dasm_flags": "-D ENDIAN_LE -D P64 -D JIT -D FFI -D FPU -D HFABI -D VER=",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_x64",
            "LUAJIT_OS=LUAJIT_OS_OSX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "machasm",
    },
    "x64_windows": {
        "dasc": "src/vm_x64.dasc",
        "dasm_flags": "-D ENDIAN_LE -D P64 -D JIT -D FFI -D FPU -D HFABI -D VER= -D WIN",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_x64",
            "LUAJIT_OS=LUAJIT_OS_WINDOWS",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "peobj",
    },
    # 32-bit targets need 32-bit host tools: buildvm's data layout mirrors the
    # target's pointer size (the Makefile's "pointer size mismatch" check).
    "x86_linux": {
        "host_copts": ["-m32"],
        "dasc": "src/vm_x86.dasc",
        "dasm_flags": "-D ENDIAN_LE -D JIT -D FFI -D FPU -D HFABI -D VER=",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_x86",
            "LUAJIT_OS=LUAJIT_OS_LINUX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "elfasm",
    },
    "arm64_linux": {
        "dasc": "src/vm_arm64.dasc",
        "dasm_flags": "-D ENDIAN_LE -D P64 -D JIT -D FFI -D DUALNUM -D FPU -D HFABI -D VER=80",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_arm64",
            "LUAJIT_OS=LUAJIT_OS_LINUX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "elfasm",
    },
    "arm64_osx": {
        "dasc": "src/vm_arm64.dasc",
        "dasm_flags": "-D ENDIAN_LE -D P64 -D JIT -D FFI -D DUALNUM -D FPU -D HFABI -D VER=80",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_arm64",
            "LUAJIT_OS=LUAJIT_OS_OSX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "machasm",
    },
    # MSVC has no -m32; the 32-bit buildvm is built in the target config instead
    # (host_in_target_config) and runs on the x64 runner via WoW64.
    "x86_windows": {
        "host_in_target_config": True,
        "dasc": "src/vm_x86.dasc",
        "dasm_flags": "-D ENDIAN_LE -D JIT -D FFI -D FPU -D HFABI -D VER= -D WIN",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_x86",
            "LUAJIT_OS=LUAJIT_OS_WINDOWS",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=0",
        ],
        "vm": "peobj",
    },
    # android_armeabi_v7a: hard FPU (vfpv3), soft-float ABI, no HFABI dasm flag.
    "arm_linux": {
        "host_copts": ["-m32"],
        "dasc": "src/vm_arm.dasc",
        "dasm_flags": "-D ENDIAN_LE -D JIT -D FFI -D DUALNUM -D FPU -D VER=70",
        "defines": [
            "LUAJIT_TARGET=LUAJIT_ARCH_arm",
            "LUAJIT_OS=LUAJIT_OS_LINUX",
            "LJ_ARCH_HASFPU=1",
            "LJ_ABI_SOFTFP=1",
        ],
        "vm": "elfasm",
    },
}

_PLATFORM_VARIANT = {
    ":_windows": "x64_windows",
    ":_windows_x86": "x86_windows",
    ":_macos_arm64": "arm64_osx",
    ":_macos_x86_64": "x64_osx",
    ":_linux_x86": "x86_linux",
    ":_linux_x86_64": "x64_linux",
    ":_linux_arm64": "arm64_linux",
    ":_android_arm64": "arm64_linux",
    ":_android_armv7": "arm_linux",
    ":_android_x86_64": "x64_linux",
    ":_android_x86": "x86_linux",
}

# Library files parsed by buildvm; order matters and matches src/Makefile LJLIB_C.
_LJLIB_C = [
    "src/lib_base.c",
    "src/lib_math.c",
    "src/lib_bit.c",
    "src/lib_string.c",
    "src/lib_table.c",
    "src/lib_io.c",
    "src/lib_os.c",
    "src/lib_package.c",
    "src/lib_debug.c",
    "src/lib_jit.c",
    "src/lib_ffi.c",
    "src/lib_buffer.c",
]

# ─── Host tools ──────────────────────────────────────────────────────────────

cc_binary(
    name = "minilua",
    srcs = ["src/host/minilua.c"],
    linkopts = select({
        "@platforms//os:windows": [],
        "//conditions:default": ["-lm"],
    }),
)

# Host headers that buildvm includes from src/ (lua.h, lj_*.h, ...).
cc_library(
    name = "_host_headers",
    hdrs = glob([
        "src/*.h",
        "src/host/*.h",
        "dynasm/*.h",
    ]),
    includes = ["src"],
    deps = [":_luajit_h_lib"],
)

cc_library(
    name = "_luajit_h_lib",
    hdrs = ["gen_version/luajit.h"],
    includes = ["gen_version"],
)

[genrule(
    name = "_buildvm_arch_h_" + variant,
    srcs = glob([
        "dynasm/*.lua",
        "src/vm_*.dasc",
    ]),
    outs = ["gen_arch_%s/buildvm_arch.h" % variant],
    cmd = "$(execpath :minilua) $(execpath dynasm/dynasm.lua) -LN %s -o $@ $(execpath %s)" % (
        cfg["dasm_flags"],
        cfg["dasc"],
    ),
    tools = [":minilua"],
) for variant, cfg in _VARIANTS.items()]

[cc_library(
    name = "_buildvm_arch_" + variant,
    hdrs = ["gen_arch_%s/buildvm_arch.h" % variant],
    includes = ["gen_arch_" + variant],
) for variant in _VARIANTS]

[cc_binary(
    name = "_buildvm_" + variant,
    srcs = glob([
        "src/host/buildvm*.c",
        "src/host/buildvm*.h",
    ]),
    copts = cfg.get("host_copts", []),
    linkopts = cfg.get("host_copts", []),
    local_defines = cfg["defines"],
    deps = [
        ":_buildvm_arch_" + variant,
        ":_host_headers",
    ],
) for variant, cfg in _VARIANTS.items()]

# ─── Generated headers & VM ──────────────────────────────────────────────────

[genrule(
    name = "_gen_" + variant,
    # host_in_target_config: buildvm goes in srcs (target config) instead of
    # tools (exec config).
    srcs = _LJLIB_C + ["src/lj_opt_fold.c"] + glob(["src/*.h"]) +
           ([":_buildvm_" + variant] if cfg.get("host_in_target_config") else []),
    outs = [
        "gen_%s/lj_bcdef.h" % variant,
        "gen_%s/lj_ffdef.h" % variant,
        "gen_%s/lj_libdef.h" % variant,
        "gen_%s/lj_recdef.h" % variant,
        "gen_%s/lj_folddef.h" % variant,
        "gen_{}/lj_vm.{}".format(
            variant,
            "obj" if cfg["vm"] == "peobj" else "S",
        ),
    ],
    cmd = " && ".join([
        "BV=$(execpath :_buildvm_%s)" % variant,
        "LIBS='%s'" % " ".join(["$(execpath %s)" % c for c in _LJLIB_C]),
        "$$BV -m bcdef -o $(RULEDIR)/gen_%s/lj_bcdef.h $$LIBS" % variant,
        "$$BV -m ffdef -o $(RULEDIR)/gen_%s/lj_ffdef.h $$LIBS" % variant,
        "$$BV -m libdef -o $(RULEDIR)/gen_%s/lj_libdef.h $$LIBS" % variant,
        "$$BV -m recdef -o $(RULEDIR)/gen_%s/lj_recdef.h $$LIBS" % variant,
        "$$BV -m folddef -o $(RULEDIR)/gen_%s/lj_folddef.h $(execpath src/lj_opt_fold.c)" % variant,
        "$$BV -m {} -o $(RULEDIR)/gen_{}/lj_vm.{}".format(
            cfg["vm"],
            variant,
            "obj" if cfg["vm"] == "peobj" else "S",
        ),
    ]),
    tools = [] if cfg.get("host_in_target_config") else [":_buildvm_" + variant],
) for variant, cfg in _VARIANTS.items()]

[cc_library(
    name = "_gen_headers_" + variant,
    hdrs = [
        "gen_%s/lj_bcdef.h" % variant,
        "gen_%s/lj_ffdef.h" % variant,
        "gen_%s/lj_libdef.h" % variant,
        "gen_%s/lj_recdef.h" % variant,
        "gen_%s/lj_folddef.h" % variant,
    ],
    includes = ["gen_" + variant],
) for variant in _VARIANTS]

# luajit.h is generated from luajit_rolling.h + the release timestamp that
# GitHub's archive export substitutes into .relver. Identical for all variants.
genrule(
    name = "_luajit_h",
    srcs = [
        "src/host/genversion.lua",
        "src/luajit_rolling.h",
        ".relver",
    ],
    outs = ["gen_version/luajit.h"],
    cmd = "$(execpath :minilua) $(execpath src/host/genversion.lua) " +
          "$(execpath src/luajit_rolling.h) $(execpath .relver) $@",
    tools = [":minilua"],
)

# ─── The library ─────────────────────────────────────────────────────────────

cc_library(
    name = "luajit",
    srcs = glob(
        [
            "src/lj_*.c",
            "src/lib_*.c",
            "src/*.h",
        ],
        exclude = ["src/ljamalg.c"],
    ) + select({
        platform: ["gen_{}/lj_vm.{}".format(
            v,
            "obj" if _VARIANTS[v]["vm"] == "peobj" else "S",
        )]
        for platform, v in _PLATFORM_VARIANT.items()
    }),
    hdrs = [
        "gen_version/luajit.h",
        "src/lauxlib.h",
        "src/lua.h",
        "src/lua.hpp",
        "src/luaconf.h",
        "src/lualib.h",
    ],
    copts = select({
        # External frame unwinding, as auto-detected by src/Makefile on all
        # modern non-Windows toolchains. Fortify and stack protectors are
        # disabled the same way luajit-cmake does it - distro compilers enable
        # them by default, and upstream deliberately builds without.
        "@platforms//os:windows": [],
        "//conditions:default": [
            "-DLUAJIT_UNWIND_EXTERNAL",
            "-fomit-frame-pointer",
            "-U_FORTIFY_SOURCE",
            "-fno-stack-protector",
        ],
    }),
    includes = [
        "gen_version",
        "src",
    ],
    linkopts = select({
        "@platforms//os:windows": [],
        "@platforms//os:android": [],
        "@platforms//os:macos": [],
        "//conditions:default": [
            "-lm",
            "-ldl",
        ],
    }),
    # Lua 5.2 compat (__pairs etc.) - sol2 and our scripts rely on it; matches
    # luajit-cmake's default. Library-only, same as the cmake build.
    local_defines = ["LUAJIT_ENABLE_LUA52COMPAT"],
    visibility = ["//visibility:public"],
    deps = select({
        platform: [":_gen_headers_" + v]
        for platform, v in _PLATFORM_VARIANT.items()
    }),
)
