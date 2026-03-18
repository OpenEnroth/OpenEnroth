# Injected BUILD file for Android prebuilt dep archives.
# Same posix layout — lib/lib*.a, include/
# OpenAL and OpenGL are provided by the Android NDK/system, not included here.

cc_import(name = "_sdl3_lib",      static_library = "lib/libSDL3.a",      hdrs = glob(["include/SDL3/**"]))
cc_library(name = "sdl3",          includes = ["include"], deps = [":_sdl3_lib"],      visibility = ["//visibility:public"])

cc_import(name = "_zlib_lib",      static_library = "lib/libz.a",         hdrs = ["include/zlib.h", "include/zconf.h"])
cc_library(name = "zlib",          includes = ["include"], deps = [":_zlib_lib"],      visibility = ["//visibility:public"])

cc_import(name = "_png_lib",       static_library = "lib/libpng16.a",     hdrs = glob(["include/png*.h"]))
cc_library(name = "png",           includes = ["include"], deps = [":_png_lib", ":zlib"], visibility = ["//visibility:public"])

cc_import(name = "_avcodec_lib",   static_library = "lib/libavcodec.a",   hdrs = glob(["include/libavcodec/**"]))
cc_import(name = "_avformat_lib",  static_library = "lib/libavformat.a",  hdrs = glob(["include/libavformat/**"]))
cc_import(name = "_avutil_lib",    static_library = "lib/libavutil.a",    hdrs = glob(["include/libavutil/**"]))
cc_import(name = "_swscale_lib",   static_library = "lib/libswscale.a",   hdrs = glob(["include/libswscale/**"]))
cc_import(name = "_swresample_lib",static_library = "lib/libswresample.a",hdrs = glob(["include/libswresample/**"]))

cc_library(name = "avcodec",    includes = ["include"], deps = [":_avcodec_lib"],    visibility = ["//visibility:public"])
cc_library(name = "avformat",   includes = ["include"], deps = [":_avformat_lib"],   visibility = ["//visibility:public"])
cc_library(name = "avutil",     includes = ["include"], deps = [":_avutil_lib"],     visibility = ["//visibility:public"])
cc_library(name = "swscale",    includes = ["include"], deps = [":_swscale_lib"],    visibility = ["//visibility:public"])
cc_library(name = "swresample", includes = ["include"], deps = [":_swresample_lib"], visibility = ["//visibility:public"])

cc_library(
    name = "ffmpeg",
    includes = ["include"],
    deps = [":_avcodec_lib", ":_avformat_lib", ":_avutil_lib", ":_swscale_lib", ":_swresample_lib"],
    visibility = ["//visibility:public"],
)

# OpenAL on Android is provided by the NDK -- stub that links against the system lib.
cc_library(
    name = "openal",
    linkopts = ["-lOpenSLES"],
    visibility = ["//visibility:public"],
)
