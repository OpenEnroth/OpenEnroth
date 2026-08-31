load("@rules_cc//cc:cc_library.bzl", "cc_library")

# Configured the way cmake configures it: the unwinder the C++ runtime already links on posix, libdwarf for
# symbols, dbghelp for everything on windows. Cpptrace's cmake probes the platform for the HAS_* macros, here
# they're stated - every posix target has dl_find_object or mach_vm and a compiler that packs structs.
cc_library(
    name = "cpptrace",
    srcs = glob([
        "src/**/*.cpp",
        "src/**/*.hpp",
    ]),
    hdrs = glob([
        "include/cpptrace/*.hpp",
        "include/ctrace/*.h",
    ]),
    defines = ["CPPTRACE_STATIC_DEFINE"],
    includes = [
        "include",
        "src",
    ],
    linkopts = select({
        "@platforms//os:windows": ["dbghelp.lib"],
        "//conditions:default": [],
    }),
    local_defines = ["NOMINMAX"] + select({
        "@platforms//os:windows": [
            "CPPTRACE_DEMANGLE_WITH_WINAPI",
            "CPPTRACE_GET_SYMBOLS_WITH_DBGHELP",
            "CPPTRACE_UNWIND_WITH_DBGHELP",
        ],
        "@platforms//os:linux": [
            "CPPTRACE_DEMANGLE_WITH_CXXABI",
            "CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF",
            "CPPTRACE_UNWIND_WITH_UNWIND",
            "CPPTRACE_HAS_DL_FIND_OBJECT",
            "CPPTRACE_HAS_CXX_EXCEPTION_TYPE",
            "HAS_ATTRIBUTE_PACKED",
        ],
        "@platforms//os:macos": [
            "CPPTRACE_DEMANGLE_WITH_CXXABI",
            "CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF",
            "CPPTRACE_UNWIND_WITH_UNWIND",
            "CPPTRACE_HAS_MACH_VM",
            "CPPTRACE_HAS_CXX_EXCEPTION_TYPE",
            "HAS_ATTRIBUTE_PACKED",
        ],
    }),
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:windows": [],
        "//conditions:default": ["@libdwarf"],
    }),
)
