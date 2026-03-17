# backward-cpp — stack trace library. Not used on Android.
cc_library(
    name = "backward_cpp",
    srcs = ["backward.cpp"],
    hdrs = ["backward.hpp"],
    includes = ["."],
    # On Linux, libdwarf improves stack trace quality. Linked externally.
    linkopts = select({
        "@platforms//os:linux": ["-ldwarf", "-lelf"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
