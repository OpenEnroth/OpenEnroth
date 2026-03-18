cc_library(
    name = "ztd_text",
    hdrs = glob(["include/ztd/**/*.hpp", "include/ztd/**/*.h"], allow_empty = True),
    includes = ["include"],
    deps = [
        "@@+_repo_rules+ztd_idk//:ztd_idk",
        "@@+_repo_rules+ztd_platform//:ztd_platform",
        "@@+_repo_rules+ztd_encoding_tables//:ztd_encoding_tables",
        "@@+_repo_rules+ztd_cuneicode//:ztd_cuneicode",
        "@@+_repo_rules+ztd_inline_containers//:ztd_inline_containers",
    ],
    visibility = ["//visibility:public"],
)
