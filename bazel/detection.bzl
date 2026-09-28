# Compile-time build identification, mirroring what cmake computes in
# CMakeModules/Detection.cmake. Used by the engine targets in src/Engine.
# PROJECT_VERSION is not here - it changes per commit, so it comes from the
# stamp genrule in //src/Library/BuildInfo instead of a define.

OE_BUILD_DEFINES = select({
    "@platforms//os:windows": ['OE_BUILD_PLATFORM=\\"windows\\"'],
    "@platforms//os:macos": ['OE_BUILD_PLATFORM=\\"darwin\\"'],
    "@platforms//os:linux": ['OE_BUILD_PLATFORM=\\"linux\\"'],
    "@platforms//os:android": ['OE_BUILD_PLATFORM=\\"android\\"'],
    "//conditions:default": ['OE_BUILD_PLATFORM=\\"unknown\\"'],
}) + select({
    "//bazel:cpu_x86_32_shim": ['OE_BUILD_ARCHITECTURE=\\"x86\\"'],
    "//bazel:cpu_x86_64": ['OE_BUILD_ARCHITECTURE=\\"x86_64\\"'],
    "@platforms//cpu:x86_32": ['OE_BUILD_ARCHITECTURE=\\"x86\\"'],
    "@platforms//cpu:arm64": ['OE_BUILD_ARCHITECTURE=\\"arm64\\"'],
    "@platforms//cpu:armv7": ['OE_BUILD_ARCHITECTURE=\\"arm32\\"'],
    "//conditions:default": ['OE_BUILD_ARCHITECTURE=\\"unknown\\"'],
}) + select({
    # Cmake reports AppleClang on macos - close enough to not special-case.
    "@rules_cc//cc/compiler:gcc": ['OE_BUILD_COMPILER=\\"GNU\\"'],
    "@rules_cc//cc/compiler:clang": ['OE_BUILD_COMPILER=\\"Clang\\"'],
    "@rules_cc//cc/compiler:clang-cl": ['OE_BUILD_COMPILER=\\"Clang\\"'],
    "@rules_cc//cc/compiler:msvc-cl": ['OE_BUILD_COMPILER=\\"MSVC\\"'],
    "//conditions:default": ['OE_BUILD_COMPILER=\\"unknown\\"'],
})
