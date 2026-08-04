"""Hermetic build tools for windows: MSYS2 (make, pkgconf) and cmake.

Windows builds must not require anything beyond msvc + bazel + git, so the
make/pkg-config/cmake used by rules_foreign_cc are downloaded here instead of
taken from the machine. ffmpeg's out-of-tree build needs an MSYS make: native
Win32 makes can't resolve the msys-style paths in its generated stub Makefile.
"""

_MSYS2_BASE_URL = "https://github.com/msys2/msys2-installer/releases/download/2026-06-11/msys2-base-x86_64-20260611.tar.xz"
_MSYS2_BASE_SHA256 = "a2d047e8ee213c3c6a49a8de427eb1069df12207c0422ff1b3cbb5c905c34221"

# The msys2 mirrors prune superseded packages; bump versions when a URL dies.
_MSYS2_PACKAGES = [
    ("make-4.4.1-3-x86_64.pkg.tar.zst", ""),
    ("pkgconf-3.0.5-1-x86_64.pkg.tar.zst", ""),
]

_CMAKE_URL = "https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-windows-x86_64.zip"
_CMAKE_SHA256 = "d163cd3ab4959b0a53fa8988f2ddbd2e6c501658201e6a154386bad9dbe4f836"
_CMAKE_STRIP_PREFIX = "cmake-3.31.6-windows-x86_64"

_HEADER = """\
load("@rules_foreign_cc//toolchains/native_tools:native_tools_toolchain.bzl", "native_tool_toolchain")

package(default_visibility = ["//visibility:public"])
"""

_TOOLCHAIN_TMPL = """
native_tool_toolchain(
    name = "{name}_tool",
    path = "{path}",
)

toolchain(
    name = "{name}_toolchain",
    exec_compatible_with = ["@platforms//os:windows"],
    toolchain = ":{name}_tool",
    toolchain_type = "@rules_foreign_cc//toolchains:{type}",
)
"""

def _msys2_impl(ctx):
    if ctx.os.name.startswith("windows"):
        ctx.download_and_extract(
            url = _MSYS2_BASE_URL,
            sha256 = _MSYS2_BASE_SHA256,
            stripPrefix = "msys64",
        )
        for pkg, sha256 in _MSYS2_PACKAGES:
            ctx.download_and_extract(
                url = [
                    "https://mirror.msys2.org/msys/x86_64/" + pkg,
                    "https://repo.msys2.org/msys/x86_64/" + pkg,
                ],
                sha256 = sha256,
            )
        root = str(ctx.path(""))
    else:
        root = "/msys2-is-windows-only"
    ctx.file("BUILD.bazel", _HEADER + _TOOLCHAIN_TMPL.format(
        name = "make",
        path = root + "/usr/bin/make.exe",
        type = "make_toolchain",
    ) + _TOOLCHAIN_TMPL.format(
        name = "pkgconfig",
        path = root + "/usr/bin/pkgconf.exe",
        type = "pkgconfig_toolchain",
    ))

def _cmake_windows_impl(ctx):
    if ctx.os.name.startswith("windows"):
        ctx.download_and_extract(
            url = _CMAKE_URL,
            sha256 = _CMAKE_SHA256,
            stripPrefix = _CMAKE_STRIP_PREFIX,
        )
        root = str(ctx.path(""))
    else:
        root = "/cmake-is-windows-only"
    ctx.file("BUILD.bazel", _HEADER + _TOOLCHAIN_TMPL.format(
        name = "cmake",
        path = root + "/bin/cmake.exe",
        type = "cmake_toolchain",
    ))

_msys2 = repository_rule(implementation = _msys2_impl)
_cmake_windows = repository_rule(implementation = _cmake_windows_impl)

def _windows_tools_impl(_ctx):
    _msys2(name = "msys2")
    _cmake_windows(name = "cmake_windows")

windows_tools = module_extension(implementation = _windows_tools_impl)
