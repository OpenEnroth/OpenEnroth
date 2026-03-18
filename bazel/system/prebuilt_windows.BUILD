# Injected BUILD file for Windows prebuilt dep archives.
# Archive layout: include/ + lib/*.lib
#
# Pattern: cc_import handles the .lib, cc_library wrapper propagates includes.
# cc_import's includes= attribute doesn't propagate to transitive deps in Bazel,
# so a cc_library wrapper with includes= is required.

cc_import(
    name = "_sdl3_lib",
    static_library = "lib/SDL3-static.lib",
    hdrs = glob(["include/SDL3/**"]),
)
cc_library(
    name = "sdl3",
    includes = ["include"],
    linkopts = [
        "kernel32.lib", "user32.lib", "gdi32.lib", "winmm.lib",
        "imm32.lib", "ole32.lib", "oleaut32.lib", "version.lib",
        "uuid.lib", "advapi32.lib", "setupapi.lib", "shell32.lib", "dinput8.lib",
    ],
    deps = [":_sdl3_lib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "_openal_lib",
    static_library = "lib/OpenAL32.lib",
    hdrs = glob(["include/AL/**"]),
)
cc_library(
    name = "openal",
    includes = ["include/AL"],
    defines = ["AL_LIBTYPE_STATIC"],  # Disable dllimport; OpenAL32.lib is a static library
    linkopts = ["avrt.lib", "winmm.lib", "ole32.lib"],
    deps = [":_openal_lib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "_zlib_lib",
    static_library = "lib/zlibstatic.lib",
    hdrs = ["include/zlib.h", "include/zconf.h"],
)
cc_library(
    name = "zlib",
    includes = ["include"],
    deps = [":_zlib_lib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "_png_lib",
    static_library = "lib/libpng16_static.lib",
    hdrs = glob(["include/png*.h", "include/libpng16/*.h"]),
)
cc_library(
    name = "png",
    includes = ["include"],
    deps = [":_png_lib", ":zlib"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "_avcodec_lib",
    static_library = "lib/avcodec.lib",
    hdrs = glob(["include/libavcodec/**"]),
)
cc_import(
    name = "_avformat_lib",
    static_library = "lib/avformat.lib",
    hdrs = glob(["include/libavformat/**"]),
)
cc_import(
    name = "_avutil_lib",
    static_library = "lib/avutil.lib",
    hdrs = glob(["include/libavutil/**"]),
)
cc_import(
    name = "_swscale_lib",
    static_library = "lib/swscale.lib",
    hdrs = glob(["include/libswscale/**"]),
)
cc_import(
    name = "_swresample_lib",
    static_library = "lib/swresample.lib",
    hdrs = glob(["include/libswresample/**"]),
)
cc_library(
    name = "ffmpeg",
    includes = ["include"],
    deps = [
        ":_avcodec_lib",
        ":_avformat_lib",
        ":_avutil_lib",
        ":_swscale_lib",
        ":_swresample_lib",
    ],
    visibility = ["//visibility:public"],
)

# Convenience aliases matching the individual ffmpeg lib names expected by //bazel/system:BUILD
cc_library(name = "avcodec",    includes = ["include"], deps = [":_avcodec_lib"],    visibility = ["//visibility:public"])
cc_library(name = "avformat",   includes = ["include"], deps = [":_avformat_lib"],   visibility = ["//visibility:public"])
cc_library(name = "avutil",     includes = ["include"], deps = [":_avutil_lib"],     visibility = ["//visibility:public"])
cc_library(name = "swscale",    includes = ["include"], deps = [":_swscale_lib"],    visibility = ["//visibility:public"])
cc_library(name = "swresample", includes = ["include"], deps = [":_swresample_lib"], visibility = ["//visibility:public"])
