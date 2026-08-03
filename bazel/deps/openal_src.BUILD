# cmake() build for OpenAL-soft 1.24.3 from source.
# Replaces the prebuilt OpenAL from OpenEnroth_Dependencies.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")


# The bazel-generated crosstool passes the NDK's raw clang without a --target
# triple, so CMake's compile/link probes produce host objects. Supply the triple
# per ABI in compile & link flags - NDK clang finds its sysroot from the triple.
# linux_x86: the blanked CMAKE_*_FLAGS below also strip bazel's -m32; restore it.
config_setting(
    name = "_linux_x86",
    constraint_values = ["@platforms//os:linux", "@platforms//cpu:x86_64"],
    define_values = {"oe_build_arch": "x86_32"},
)

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


filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "openal_desktop",
    lib_source = ":all_srcs",
    out_static_libs = select({
        "@platforms//os:windows": ["OpenAL32.lib"],
        "//conditions:default": ["libopenal.a"],
    }),
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # Use static MSVC runtime (/MT) to match the rest of the build.
        "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
        "LIBTYPE": "STATIC",
        "ALSOFT_UTILS": "OFF",
        "ALSOFT_EXAMPLES": "OFF",
        "ALSOFT_TESTS": "OFF",
        # Suppress a CMP0048 warning about project() not specifying version.
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
        # CMP0091 (NEW): cmake manages MSVC runtime library selection via
        # CMAKE_MSVC_RUNTIME_LIBRARY. Without this, cmake's Release config
        # adds /MD which overrides the /MT we set above.
        "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
        # rules_foreign_cc unconditionally exports a CXXFLAGS env var containing
        # all Bazel --copt flags (e.g. /Zc:preprocessor, /Isrc, /Itest). cmake picks
        # up CXXFLAGS as the initial value of CMAKE_CXX_FLAGS. Explicitly set them
        # empty here so OpenAL-soft compiles with only its own cmake-configured flags.
        # Use a single space to pass -DCMAKE_CXX_FLAGS= to cmake configure, overriding
        # the CXXFLAGS env var that rules_foreign_cc exports with all Bazel --copt flags.
        # (cmake filters empty-string cache entries, so we use a space as a no-op value.)
        "CMAKE_CXX_FLAGS": " ",
        "CMAKE_C_FLAGS": " ",
    },
    # openal-soft sets -DNTDDI_VERSION=NTDDI_VISTA (0x06000000) but not _WIN32_WINNT.
    # Windows SDK 10.0.26100 requires both to be defined consistently; without
    # _WIN32_WINNT, sdkddkver.h(302) errors: "NTDDI_VERSION setting conflicts with
    # _WIN32_WINNT setting". Add _WIN32_WINNT=0x0600 (Vista) via generate_args so
    # it overrides the space-valued CMAKE_C_FLAGS from cache_entries above
    # (cmake uses the last -D value when the same variable is set multiple times).
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
    # Don't generate a Bazel crosstool cmake toolchain file. The default crosstool
    # injects Bazel's --copt flags which conflict with OpenAL-soft's own compile
    # settings. cmake auto-detects the MSVC compiler from the build environment.
    generate_crosstool_file = False,
    # The prebuilt OpenAL used includes=["include/AL"] so code does #include <al.h>.
    # Match that layout by exposing include/AL as the include root instead of include/.
    out_include_dir = "include/AL",
    # AL_LIBTYPE_STATIC suppresses dllimport decorations in al.h headers.
    defines = ["AL_LIBTYPE_STATIC"],
    linkopts = select({
        "@platforms//os:windows": ["avrt.lib", "winmm.lib", "ole32.lib"],
        # OpenAL-soft on macOS links against system audio frameworks. cmake() builds
        # the static lib but doesn't propagate these transitive link deps to Bazel;
        # add them explicitly so downstream targets link successfully.
        # Use -Wl,-framework,Name (single string) to avoid two-entry pair ordering
        # issues in Bazel 8+ linkopts handling.
        "@platforms//os:macos": [
            "-Wl,-framework,CoreAudio",
            "-Wl,-framework,AudioUnit",
            "-Wl,-framework,CoreFoundation",
            "-Wl,-framework,AudioToolbox",
        ],
        "//conditions:default": [],
    }),
)

# Android cross-compilation needs the bazel-generated crosstool file so cmake uses the
# NDK toolchain that bazel resolved (with generate_crosstool_file = False it would
# auto-detect the host compiler). Desktop platforms keep the crosstool off — see above.
cmake(
    name = "openal_android",
    lib_source = ":all_srcs",
    out_static_libs = ["libopenal.a"],
    generate_args = select({
        ":_android_arm64": [
        "-DCMAKE_C_FLAGS=--target=aarch64-linux-android24",
        "-DCMAKE_CXX_FLAGS=--target=aarch64-linux-android24",
        "-DCMAKE_ASM_FLAGS=--target=aarch64-linux-android24",
        "-DCMAKE_EXE_LINKER_FLAGS=--target=aarch64-linux-android24",
    ],
        ":_android_armv7": [
        "-DCMAKE_C_FLAGS=--target=armv7a-linux-androideabi24",
        "-DCMAKE_CXX_FLAGS=--target=armv7a-linux-androideabi24",
        "-DCMAKE_ASM_FLAGS=--target=armv7a-linux-androideabi24",
        "-DCMAKE_EXE_LINKER_FLAGS=--target=armv7a-linux-androideabi24",
    ],
        ":_android_x86_64": [
        "-DCMAKE_C_FLAGS=--target=x86_64-linux-android24",
        "-DCMAKE_CXX_FLAGS=--target=x86_64-linux-android24",
        "-DCMAKE_ASM_FLAGS=--target=x86_64-linux-android24",
        "-DCMAKE_EXE_LINKER_FLAGS=--target=x86_64-linux-android24",
    ],
        "//conditions:default": [],
    }),
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        "LIBTYPE": "STATIC",
        "ALSOFT_UTILS": "OFF",
        "ALSOFT_EXAMPLES": "OFF",
        "ALSOFT_TESTS": "OFF",
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
    },
    out_include_dir = "include/AL",
    defines = ["AL_LIBTYPE_STATIC"],
    linkopts = ["-lOpenSLES"],
)

cc_library(
    name = "openal",
    deps = select({
        "@platforms//os:android": [":openal_android"],
        "//conditions:default": [":openal_desktop"],
    }),
    visibility = ["//visibility:public"],
)
