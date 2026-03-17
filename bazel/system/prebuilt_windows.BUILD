# Injected BUILD file for Windows prebuilt dep archives.
# Archive layout: include/ + lib/*.lib

cc_import(
    name = "sdl3",
    static_library = "lib/SDL3-static.lib",
    hdrs = glob(["include/SDL3/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "openal",
    static_library = "lib/OpenAL32.lib",
    hdrs = glob(["include/AL/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "zlib",
    static_library = "lib/zlibstatic.lib",
    hdrs = ["include/zlib.h", "include/zconf.h"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "png",
    static_library = "lib/libpng16_static.lib",
    hdrs = glob(["include/png*.h", "include/libpng*.h"]),
    includes = ["include"],
    deps = [":zlib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avcodec",
    static_library = "lib/avcodec.lib",
    hdrs = glob(["include/libavcodec/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avformat",
    static_library = "lib/avformat.lib",
    hdrs = glob(["include/libavformat/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avutil",
    static_library = "lib/avutil.lib",
    hdrs = glob(["include/libavutil/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "swscale",
    static_library = "lib/swscale.lib",
    hdrs = glob(["include/libswscale/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "swresample",
    static_library = "lib/swresample.lib",
    hdrs = glob(["include/libswresample/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ffmpeg",
    deps = [":avcodec", ":avformat", ":avutil", ":swscale", ":swresample"],
    visibility = ["//visibility:public"],
)
