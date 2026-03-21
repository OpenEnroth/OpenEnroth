# cmake() build for libpng 1.6.44 from source.
# Replaces the prebuilt libpng from OpenEnroth_Dependencies.
# Depends on @zlib_src//:zlib — must be migrated together with zlib.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

cmake(
    name = "png",
    lib_source = ":all_srcs",
    out_static_libs = select({
        "@platforms//os:windows": ["libpng16_static.lib"],
        "//conditions:default": ["libpng16.a"],
    }),
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        # Use static MSVC runtime (/MT) to match the rest of the build.
        "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded",
        "PNG_STATIC": "ON",
        "PNG_SHARED": "OFF",
        "PNG_TESTS": "OFF",
        "PNG_TOOLS": "OFF",
        # Suppress a CMP0048 warning about project() not specifying version.
        "CMAKE_POLICY_VERSION_MINIMUM": "3.5",
        # CMP0091 (NEW): cmake manages MSVC runtime library selection via
        # CMAKE_MSVC_RUNTIME_LIBRARY. Without this, cmake's Release config
        # adds /MD which overrides the /MT we set above.
        "CMAKE_POLICY_DEFAULT_CMP0091": "NEW",
    },
    # rules_foreign_cc sets CMAKE_PREFIX_PATH=$EXT_BUILD_DEPS (the outer dir).
    # cmake's FindZLIB.cmake won't find zlib there because the actual zlib install
    # is at $EXT_BUILD_DEPS/zlib/. Pass ZLIB_ROOT explicitly so find_package works.
    # The zlib postfix_script creates zlib.lib (an alias for zlibstatic.lib) since
    # FindZLIB.cmake searches for names "z", "zlib", "zdll", "zlib_static" — not "zlibstatic".
    generate_args = ["-DZLIB_ROOT=$EXT_BUILD_DEPS/zlib"],
    deps = ["@zlib_src//:zlib"],
    visibility = ["//visibility:public"],
)
