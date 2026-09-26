"""Hermetic build tools for windows: MSYS2 make and pkgconf.

Windows builds must not require anything beyond msvc + bazel + git, so the
make/pkg-config used by rules_foreign_cc are downloaded here instead of taken
from the machine (cmake/ninja come prebuilt from rules_foreign_cc itself).
ffmpeg's out-of-tree build needs an MSYS make: native Win32 makes can't
resolve the msys-style paths in its generated stub Makefile.
"""

_MSYS2_BASE_URL = "https://github.com/msys2/msys2-installer/releases/download/2026-06-11/msys2-base-x86_64-20260611.tar.xz"
_MSYS2_BASE_SHA256 = "a2d047e8ee213c3c6a49a8de427eb1069df12207c0422ff1b3cbb5c905c34221"

# The msys2 mirrors prune superseded packages; bump versions when a URL dies.
# pkgconf only satisfies rules_foreign_cc's mandatory pkgconfig toolchain
# resolution; nothing in this build actually queries pkg-config.
_MSYS2_PACKAGES = [
    ("make-4.4.1-3-x86_64.pkg.tar.zst", "af0bdba17f06fe037f0194069adaa31a8fe45f1a11381501896aea1fae37bd5d"),
    ("pkgconf-3.0.5-1-x86_64.pkg.tar.zst", "40dfc37c4fed31b7bf2fe55f11299884e3120025d27f61255ad1a1c8b890aece"),
]

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

_msys2 = repository_rule(implementation = _msys2_impl)

def _windows_tools_impl(_ctx):
    _msys2(name = "msys2")

windows_tools = module_extension(implementation = _windows_tools_impl)
