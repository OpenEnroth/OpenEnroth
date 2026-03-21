# cmake() build for SDL3 3.2.22 from source.
# Replaces the prebuilt SDL3 from OpenEnroth_Dependencies.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "sdl3",
    lib_source = ":all_srcs",
    out_static_libs = select({
        "@platforms//os:windows": ["SDL3-static.lib"],
        "//conditions:default": ["libSDL3.a"],
    }),
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # Use static MSVC runtime (/MT) to match the rest of the build.
        "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
        "SDL_STATIC": "ON",
        "SDL_SHARED": "OFF",
        "SDL_TEST_LIBRARY": "OFF",
        "SDL_TESTS": "OFF",
        # Suppress a CMP0048 warning about project() not specifying version.
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
        # CMP0091 (NEW): cmake manages MSVC runtime library selection via
        # CMAKE_MSVC_RUNTIME_LIBRARY. Without this, cmake's Release config
        # adds /MD which overrides the /MT we set above.
        "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
    },
    # System libraries required by the SDL3 static build. cmake() builds the static lib
    # but doesn't propagate transitive link deps to Bazel; add them explicitly here.
    # On Linux, SDL3 links against system X11/Wayland/etc. via cmake's pkg-config detection
    # and these are captured in SDL3's installed cmake targets — not needed here.
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
        "@platforms//os:macos": [
            "-framework Cocoa",
            "-framework IOKit",
            "-framework CoreFoundation",
            "-framework CoreAudio",
            "-framework AudioToolbox",
            "-framework Metal",
            "-framework QuartzCore",
            "-framework GameController",
            "-framework CoreHaptics",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
