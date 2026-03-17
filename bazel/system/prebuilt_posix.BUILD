# Injected BUILD file for Linux and macOS prebuilt dep archives.
# Archive layout: include/ + lib/lib*.a

cc_import(
    name = "sdl3",
    static_library = "lib/libSDL3.a",
    hdrs = glob(["include/SDL3/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "openal",
    static_library = "lib/libopenal.a",
    hdrs = glob(["include/AL/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "zlib",
    static_library = "lib/libz.a",
    hdrs = ["include/zlib.h", "include/zconf.h"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "png",
    static_library = "lib/libpng16.a",
    hdrs = glob(["include/png*.h", "include/libpng*.h"]),
    includes = ["include"],
    deps = [":zlib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avcodec",
    static_library = "lib/libavcodec.a",
    hdrs = glob(["include/libavcodec/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avformat",
    static_library = "lib/libavformat.a",
    hdrs = glob(["include/libavformat/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "avutil",
    static_library = "lib/libavutil.a",
    hdrs = glob(["include/libavutil/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "swscale",
    static_library = "lib/libswscale.a",
    hdrs = glob(["include/libswscale/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "swresample",
    static_library = "lib/libswresample.a",
    hdrs = glob(["include/libswresample/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

# Convenience group for all FFmpeg libs.
cc_library(
    name = "ffmpeg",
    deps = [":avcodec", ":avformat", ":avutil", ":swscale", ":swresample"],
    visibility = ["//visibility:public"],
)
