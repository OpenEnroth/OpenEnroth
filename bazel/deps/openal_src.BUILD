# cmake() build for OpenAL-soft 1.24.3 from source.
# Replaces the prebuilt OpenAL from OpenEnroth_Dependencies.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

# The bazel-generated crosstool passes the NDK's raw clang without a --target
# triple, so CMake's compile/link probes produce host objects. Supply the triple
# per ABI in compile & link flags - NDK clang finds its sysroot from the triple.
# linux_x86: the blanked CMAKE_*_FLAGS below also strip bazel's -m32; restore it.
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

# Windows -c dbg compiles the engine /MTd; deps must be on the same debug CRT
# or the link pulls in both CRTs and fails (see the cache_entries select).
config_setting(
    name = "_windows_dbg",
    constraint_values = ["@platforms//os:windows"],
    values = {"compilation_mode": "dbg"},
)

config_setting(
    name = "_linux",
    constraint_values = ["@platforms//os:linux"],
)

filegroup(
    name = "all_srcs",
    srcs = glob(
        ["**"],
        exclude = ["BUILD.bazel"],
    ),
)

_CACHE_ENTRIES = {
    "CMAKE_BUILD_TYPE": "Release",
    # GNUInstallDirs picks lib64 on non-debian roots (e.g. the flatpak sandbox).
    "CMAKE_INSTALL_LIBDIR": "lib",
    # Use static MSVC runtime (/MT) to match the rest of the build.
    "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
    "LIBTYPE": "STATIC",
    "ALSOFT_UTILS": "OFF",
    "ALSOFT_EXAMPLES": "OFF",
    "ALSOFT_TESTS": "OFF",
    # Suppress a CMP0048 warning about project() not specifying version.
    "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
    # Without CMP0091=NEW cmake's Release config adds /MD, overriding /MT.
    "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
    # Blank out the CFLAGS/CXXFLAGS env vars rules_foreign_cc exports (all of
    # bazel's --copts); a single space because cmake drops empty cache entries.
    "CMAKE_CXX_FLAGS": " ",
    "CMAKE_C_FLAGS": " ",
}

cmake(
    name = "openal_desktop",
    # Lib names stay the same in Debug: openal-soft defaults CMAKE_DEBUG_POSTFIX
    # to an empty string.
    cache_entries = select({
        ":_windows_dbg": _CACHE_ENTRIES | {
            "CMAKE_BUILD_TYPE": "Debug",
            "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreadedDebug",
            # Cmake's msvc Debug default is /Zi, which funnels every cl.exe
            # through one mspdbsrv RPC server and flakes under parallel ninja
            # (C1090, PDB API call failed). Embedded /Z7 has no server.
            "CMAKE_C_FLAGS_DEBUG": "/Z7 /Ob0 /Od /RTC1",
            "CMAKE_CXX_FLAGS_DEBUG": "/Z7 /Ob0 /Od /RTC1",
        },
        # The backends are dlopened at runtime, so a missing dev package at
        # build time silently drops them from the binary - the prebuilt deps
        # were built with all three. REQUIRE turns that into a configure error.
        # 32-bit only gets ALSA, like the prebuilts effectively did: pulse
        # needs libpulse-dev:i386, which noble's partial i386 archive can't
        # install, and pipewire/pulse detection runs through pkg-config, which
        # never searches the i386 dirs on an amd64 host. Pulse and pipewire
        # systems route 32-bit audio through their alsa plugins.
        ":_linux_x86": _CACHE_ENTRIES | {
            "ALSOFT_REQUIRE_ALSA": "ON",
        },
        ":_linux": _CACHE_ENTRIES | {
            "ALSOFT_REQUIRE_ALSA": "ON",
            "ALSOFT_REQUIRE_PULSEAUDIO": "ON",
            "ALSOFT_REQUIRE_PIPEWIRE": "ON",
        },
        "//conditions:default": _CACHE_ENTRIES,
    }),
    # AL_LIBTYPE_STATIC suppresses dllimport decorations in al.h headers.
    defines = ["AL_LIBTYPE_STATIC"],
    # openal-soft sets NTDDI_VERSION but not _WIN32_WINNT; SDK 10.0.26100 errors
    # unless both agree. generate_args -D wins over cache_entries' blank flags.
    generate_args = select({
        "@platforms//os:windows": [
            "-DCMAKE_C_FLAGS=/D_WIN32_WINNT=0x0600",
            "-DCMAKE_CXX_FLAGS=/D_WIN32_WINNT=0x0600",
        ],
        ":_linux_x86": [
            "-DCMAKE_C_FLAGS=-m32",
            "-DCMAKE_CXX_FLAGS=-m32",
        ],
        "//conditions:default": [],
    }),
    # The crosstool toolchain file would inject all of bazel's --copts.
    generate_crosstool_file = False,
    lib_source = ":all_srcs",
    linkopts = select({
        "@platforms//os:windows": [
            "avrt.lib",
            "winmm.lib",
            "ole32.lib",
        ],
        # Single-string -Wl,-framework,Name dodges Bazel 8 linkopt pair reordering.
        "@platforms//os:macos": [
            "-Wl,-framework,CoreAudio",
            "-Wl,-framework,AudioUnit",
            "-Wl,-framework,CoreFoundation",
            "-Wl,-framework,AudioToolbox",
        ],
        "//conditions:default": [],
    }),
    # The prebuilt OpenAL exposed include/AL, so code does #include <al.h>.
    out_include_dir = "include/AL",
    out_static_libs = select({
        "@platforms//os:windows": ["OpenAL32.lib"],
        "//conditions:default": ["libopenal.a"],
    }),
)

# Android keeps the crosstool file: cmake must use the NDK toolchain bazel
# resolved, not whatever compiler it would auto-detect.
cmake(
    name = "openal_android",
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # GNUInstallDirs picks lib64 on non-debian roots (e.g. the flatpak sandbox).
        "CMAKE_INSTALL_LIBDIR": "lib",
        "LIBTYPE": "STATIC",
        # The crosstool reports CMAKE_SYSTEM_NAME=Linux, so OpenAL would enable the
        # OSS backend - bionic has no sys/soundcard.h. Use OpenSLES instead.
        "ALSOFT_BACKEND_OSS": "OFF",
        "ALSOFT_BACKEND_OPENSL": "ON",
        "ALSOFT_UTILS": "OFF",
        "ALSOFT_EXAMPLES": "OFF",
        "ALSOFT_TESTS": "OFF",
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
    },
    defines = ["AL_LIBTYPE_STATIC"],
    generate_args = select({
        ":_android_arm64": [
            "-DCMAKE_C_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=aarch64-linux-android24",
            "-DANDROID=YES",
        ],
        ":_android_armv7": [
            "-DCMAKE_C_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_CXX_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_ASM_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=armv7a-linux-androideabi24",
            "-DANDROID=YES",
        ],
        ":_android_x86_64": [
            "-DCMAKE_C_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=x86_64-linux-android24",
            "-DANDROID=YES",
        ],
        ":_android_x86": [
            "-DCMAKE_C_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=i686-linux-android24",
            "-DANDROID=YES",
        ],
        "//conditions:default": [],
    }),
    lib_source = ":all_srcs",
    linkopts = ["-lOpenSLES"],
    out_include_dir = "include/AL",
    out_static_libs = ["libopenal.a"],
)

cc_library(
    name = "openal",
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:android": [":openal_android"],
        "//conditions:default": [":openal_desktop"],
    }),
)
