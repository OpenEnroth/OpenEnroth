load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "mio",
    hdrs = glob([
        "include/mio/*.hpp",
        "include/mio/detail/*.hpp",
        "include/mio/detail/*.ipp",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
