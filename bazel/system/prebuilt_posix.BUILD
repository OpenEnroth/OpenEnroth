# Injected BUILD file for Linux and macOS prebuilt dep archives.
# Archive layout: include/ + lib/lib*.a
#
# Pattern: cc_import handles the .a, cc_library wrapper propagates includes.

cc_import(name = "_sdl3_lib",      static_library = "lib/libSDL3.a",      hdrs = glob(["include/SDL3/**"]))
cc_library(name = "sdl3",          includes = ["include"], deps = [":_sdl3_lib"],      visibility = ["//visibility:public"])

cc_import(name = "_openal_lib",    static_library = "lib/libopenal.a",    hdrs = glob(["include/AL/**"]))
cc_library(name = "openal",        includes = ["include/AL"], deps = [":_openal_lib"],    visibility = ["//visibility:public"])

cc_import(name = "_zlib_lib",      static_library = "lib/libz.a",         hdrs = ["include/zlib.h", "include/zconf.h"])
cc_library(name = "zlib",          includes = ["include"], deps = [":_zlib_lib"],      visibility = ["//visibility:public"])

cc_import(name = "_png_lib",       static_library = "lib/libpng16.a",     hdrs = glob(["include/png*.h", "include/libpng16/*.h"]))
cc_library(name = "png",           includes = ["include"], deps = [":_png_lib", ":zlib"], visibility = ["//visibility:public"])

# FFmpeg static archives have circular dependencies between avformat↔avcodec and avutil.
# alwayslink=True forces --whole-archive so gold links all symbols from each archive
# unconditionally, resolving circular refs without needing --start-group/--end-group.
cc_import(name = "_avcodec_lib",   static_library = "lib/libavcodec.a",   hdrs = glob(["include/libavcodec/**"]),   alwayslink = True)
cc_import(name = "_avformat_lib",  static_library = "lib/libavformat.a",  hdrs = glob(["include/libavformat/**"]),  alwayslink = True)
cc_import(name = "_avutil_lib",    static_library = "lib/libavutil.a",    hdrs = glob(["include/libavutil/**"]),    alwayslink = True)
cc_import(name = "_swscale_lib",   static_library = "lib/libswscale.a",   hdrs = glob(["include/libswscale/**"]),   alwayslink = True)
cc_import(name = "_swresample_lib",static_library = "lib/libswresample.a",hdrs = glob(["include/libswresample/**"]),alwayslink = True)

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
