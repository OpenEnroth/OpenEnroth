load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "ztd_text",
    hdrs = glob(
        [
            "include/ztd/**/*.hpp",
            "include/ztd/**/*.h",
        ],
        allow_empty = True,
    ),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@ztd_cuneicode",
        "@ztd_encoding_tables",
        "@ztd_idk",
        "@ztd_inline_containers",
        "@ztd_platform",
    ],
)
