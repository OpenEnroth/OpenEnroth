load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "ztd_platform",
    hdrs = glob(
        [
            "include/**/*.hpp",
            "include/**/*.h",
        ],
        allow_empty = True,
    ),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
