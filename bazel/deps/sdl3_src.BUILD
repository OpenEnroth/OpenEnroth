# cmake() build for SDL3 3.2.22 from source.
# Replaces the prebuilt SDL3 from OpenEnroth_Dependencies.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

# The bazel-generated crosstool passes the NDK's raw clang without a --target
# triple, so CMake's compile/link probes produce host objects. Supply the triple
# per ABI in compile & link flags - NDK clang finds its sysroot from the triple.
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

# Strict specialization of _linux: linux_x86 builds on the host x86_64
# platform with -m32 and marks itself with --define (see .bazelrc).
config_setting(
    name = "_linux_x86",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    define_values = {"oe_build_arch": "x86_32"},
)

# Java side of SDL's android support, compiled into the APK.
filegroup(
    name = "android_java",
    srcs = glob(["android-project/app/src/main/java/org/libsdl/app/*.java"]),
    visibility = ["//visibility:public"],
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
    "SDL_STATIC": "ON",
    "SDL_SHARED": "OFF",
    "SDL_TEST_LIBRARY": "OFF",
    "SDL_TESTS": "OFF",
    # Suppress a CMP0048 warning about project() not specifying version.
    "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
    # Without CMP0091=NEW cmake's Release config adds /MD, overriding /MT.
    "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
}

cmake(
    name = "sdl3",
    # Lib names stay the same in Debug: SDL sets no CMAKE_DEBUG_POSTFIX.
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
        # Audio backends are dlopened at runtime, so they only exist in the
        # binary if the dev packages were present at build time - the prebuilt
        # deps were built with all of them. SDL has no per-backend REQUIRE, so
        # unlike OpenAL these stay soft, but the explicit ON documents intent
        # and re-keys the action when the entries change. X11 and Wayland need
        # no entry: SDL itself hard-errors when neither is found. No pulse on
        # 32-bit - noble's partial i386 archive can't install libpulse-dev:i386.
        ":_linux_x86": _CACHE_ENTRIES | {
            "SDL_ALSA": "ON",
        },
        ":_linux": _CACHE_ENTRIES | {
            "SDL_ALSA": "ON",
            "SDL_PULSEAUDIO": "ON",
            "SDL_PIPEWIRE": "ON",
        },
        "//conditions:default": _CACHE_ENTRIES,
    }),
    # -DANDROID=YES: rules_foreign_cc 0.15 moved this from the cmake cache into
    # its toolchain file, where SDL's android detection no longer sees it.
    generate_args = select({
        ":_android_arm64": [
            "-DCMAKE_C_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=aarch64-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=aarch64-linux-android24",
            "-DANDROID=YES",
            "-DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME",
        ],
        ":_android_armv7": [
            "-DCMAKE_C_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_CXX_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_ASM_FLAGS=--target=armv7a-linux-androideabi24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=armv7a-linux-androideabi24",
            "-DANDROID=YES",
            "-DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME",
        ],
        ":_android_x86_64": [
            "-DCMAKE_C_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=x86_64-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=x86_64-linux-android24",
            "-DANDROID=YES",
            "-DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME",
        ],
        ":_android_x86": [
            "-DCMAKE_C_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_CXX_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_ASM_FLAGS=--target=i686-linux-android24",
            "-DCMAKE_EXE_LINKER_FLAGS=--target=i686-linux-android24",
            "-DANDROID=YES",
            "-DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME",
        ],
        "//conditions:default": [],
    }),
    lib_source = ":all_srcs",
    # System libs of the static SDL3 build; cmake() doesn't propagate them.
    linkopts = select({
        "@platforms//os:windows": [
            "kernel32.lib",
            "user32.lib",
            "gdi32.lib",
            "winmm.lib",
            "imm32.lib",
            "ole32.lib",
            "oleaut32.lib",
            "version.lib",
            "uuid.lib",
            "advapi32.lib",
            "setupapi.lib",
            "shell32.lib",
            "dinput8.lib",
        ],
        # SDL3's android backends call into these NDK system libraries.
        "@platforms//os:android": [
            "-landroid",
            "-llog",
        ],
        # Single-string -Wl,-framework,Name dodges Bazel 8 linkopt pair reordering.
        "@platforms//os:macos": [
            "-Wl,-framework,Cocoa",
            "-Wl,-framework,IOKit",
            "-Wl,-framework,CoreFoundation",
            "-Wl,-framework,CoreAudio",
            "-Wl,-framework,AudioToolbox",
            "-Wl,-framework,Metal",
            "-Wl,-framework,QuartzCore",
            "-Wl,-framework,GameController",
            "-Wl,-framework,CoreHaptics",
            # Keyboard support: TIS APIs live in Carbon.
            "-Wl,-framework,Carbon",
            # Camera support (SDL_camera_coremedia.m).
            "-Wl,-framework,AVFoundation",
            "-Wl,-framework,CoreMedia",
            "-Wl,-framework,CoreVideo",
            # Clipboard support: UTType (UniformTypeIdentifiers, macOS 11+).
            "-Wl,-framework,UniformTypeIdentifiers",
            # SDL3 joystick/haptics use IOKit ForceFeedback APIs (FFCreateDevice etc.)
            "-Wl,-framework,ForceFeedback",
        ],
        "//conditions:default": [],
    }),
    out_static_libs = select({
        "@platforms//os:windows": ["SDL3-static.lib"],
        "//conditions:default": ["libSDL3.a"],
    }),
    visibility = ["//visibility:public"],
)
