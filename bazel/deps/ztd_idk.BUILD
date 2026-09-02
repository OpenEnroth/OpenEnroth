load("@rules_cc//cc:cc_library.bzl", "cc_library")

# ztd.idk — companion library for ztd.text (provides ztd/version.hpp etc.)
cc_library(
    name = "ztd_idk",
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
