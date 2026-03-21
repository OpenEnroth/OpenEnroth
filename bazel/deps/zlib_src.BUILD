# cmake() build for zlib 1.2.13 from source.
# Replaces the prebuilt zlib from OpenEnroth_Dependencies.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "zlib",
    lib_source = ":all_srcs",
    out_static_libs = select({
        "@platforms//os:windows": ["zlibstatic.lib"],
        "//conditions:default": ["libz.a"],
    }),
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # Use static MSVC runtime (/MT) to match the rest of the build.
        # Without this, cmake defaults to /MD (dynamic CRT) for Release builds,
        # conflicting with Bazel's static_link_msvcrt feature.
        "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
        # Suppress a CMP0048 warning about project() not specifying version.
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
    },
    visibility = ["//visibility:public"],
)
