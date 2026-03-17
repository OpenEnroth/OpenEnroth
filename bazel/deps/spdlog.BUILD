cc_library(
    name = "spdlog",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob(["include/spdlog/**/*.h"]),
    includes = ["include"],
    defines = [
        "SPDLOG_COMPILED_LIB",       # Use compiled (non-header-only) mode
        "SPDLOG_FMT_EXTERNAL",        # Use external fmt library
        "SPDLOG_DISABLE_DEFAULT_LOGGER",
    ] + select({
        "@platforms//os:windows": ["SPDLOG_WCHAR_SUPPORT"],
        "//conditions:default": [],
    }),
    deps = ["@fmt//:fmt"],
    visibility = ["//visibility:public"],
)
