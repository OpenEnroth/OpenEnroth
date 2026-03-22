# cmake() build for OpenAL-soft 1.24.3 from source.
# Replaces the prebuilt OpenAL from OpenEnroth_Dependencies.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "openal",
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
    visibility = ["//visibility:public"],
)
