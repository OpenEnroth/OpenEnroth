"""Repository rule that combines LuaJIT sources with the luajit-cmake build system.

LuaJIT's own Makefile requires GCC, so on Windows it's built with MSVC through the
luajit-cmake wrapper instead. The wrapper expects the LuaJIT source tree in a sibling
directory (passed via LUAJIT_DIR), so both archives are extracted into one repository
and a top-level CMakeLists.txt wires them together.
"""

def _luajit_cmake_repository_impl(rctx):
    rctx.download_and_extract(
        url = rctx.attr.luajit_url,
        sha256 = rctx.attr.luajit_sha256,
        stripPrefix = rctx.attr.luajit_strip_prefix,
        output = "luajit",
    )
    rctx.download_and_extract(
        url = rctx.attr.luajit_cmake_url,
        sha256 = rctx.attr.luajit_cmake_sha256,
        stripPrefix = rctx.attr.luajit_cmake_strip_prefix,
        output = "luajit-cmake",
    )
    rctx.file("CMakeLists.txt", """cmake_minimum_required(VERSION 3.16)
project(luajit_wrapper C)
set(LUAJIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/luajit)
add_subdirectory(luajit-cmake)
""")
    rctx.file("BUILD.bazel", rctx.read(rctx.attr.build_file))

luajit_cmake_repository = repository_rule(
    implementation = _luajit_cmake_repository_impl,
    attrs = {
        "luajit_url": attr.string(mandatory = True),
        "luajit_sha256": attr.string(mandatory = True),
        "luajit_strip_prefix": attr.string(mandatory = True),
        "luajit_cmake_url": attr.string(mandatory = True),
        "luajit_cmake_sha256": attr.string(mandatory = True),
        "luajit_cmake_strip_prefix": attr.string(mandatory = True),
        "build_file": attr.label(mandatory = True, allow_single_file = True),
    },
)
