cc_library(
    name = "fmt",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/fmt/*.h"]),
    includes = ["include"],
    # FMT_USE_NONTYPE_TEMPLATE_ARGS enables _cf literals used throughout OE.
    defines = ["FMT_USE_NONTYPE_TEMPLATE_ARGS"],
    visibility = ["//visibility:public"],
)
