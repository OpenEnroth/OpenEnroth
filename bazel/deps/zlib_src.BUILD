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
        # CMP0091 (NEW): cmake manages MSVC runtime library selection via
        # CMAKE_MSVC_RUNTIME_LIBRARY. Without this, cmake's Release config
        # adds /MD which overrides the /MT we set above.
        "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
    },
    # cmake's FindZLIB.cmake searches for lib names "z", "zlib", "zdll", "zlib_static" —
    # not "zlibstatic". Copy the installed lib to the expected name so that downstream
    # cmake deps (e.g. libpng) can find it via find_package(ZLIB REQUIRED).
    postfix_script = select({
        "@platforms//os:windows": "cp $INSTALLDIR/lib/zlibstatic.lib $INSTALLDIR/lib/zlib.lib",
        "//conditions:default": "",
    }),
    visibility = ["//visibility:public"],
)
