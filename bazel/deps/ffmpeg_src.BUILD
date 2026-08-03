# configure_make() build for FFmpeg n7.0 from source, used on all platforms.
# The BCR ffmpeg module was evaluated but can't reproduce the stripped-down
# component set below without hand-maintaining the internal component closure,
# and it hardcodes x86-64 nasm sources for every non-aarch64 CPU.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

# Local config_settings — //bazel/platforms labels can't be used in injected BUILD files.
config_setting(
    name = "_android_armv7",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:armv7"],
)

config_setting(
    name = "_android_arm64",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:arm64"],
)

config_setting(
    name = "_android_x86_64",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:x86_64"],
)

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

configure_make(
    name = "ffmpeg",
    lib_source = ":all_srcs",
    # Mirrors the stripped-down configuration from OpenEnroth_Dependencies
    # (scripts/build_all.sh): only the decoders/demuxers MM7 media needs.
    configure_options = [
        "--disable-everything",
        "--disable-gpl",
        "--disable-version3",
        "--disable-nonfree",
        "--enable-small",
        "--enable-runtime-cpudetect",
        "--disable-gray",
        "--disable-swscale-alpha",
        "--disable-programs",
        "--disable-doc",
        "--disable-htmlpages",
        "--disable-manpages",
        "--disable-podpages",
        "--disable-txtpages",
        "--disable-iconv",
        "--disable-avdevice",
        "--disable-postproc",
        "--disable-avfilter",
        "--disable-network",
        "--enable-avcodec",
        "--enable-avformat",
        "--enable-avutil",
        "--enable-swresample",
        "--enable-swscale",
        "--disable-devices",
        "--disable-encoders",
        "--disable-filters",
        "--disable-hwaccels",
        "--disable-decoders",
        "--enable-decoder=mp3*",
        "--enable-decoder=adpcm*",
        "--enable-decoder=pcm*",
        "--enable-decoder=bink",
        "--enable-decoder=binkaudio_dct",
        "--enable-decoder=binkaudio_rdft",
        "--enable-decoder=smackaud",
        "--enable-decoder=smacker",
        "--disable-muxers",
        "--disable-demuxers",
        "--enable-demuxer=mp3",
        "--enable-demuxer=bink",
        "--enable-demuxer=binka",
        "--enable-demuxer=smacker",
        "--enable-demuxer=pcm*",
        "--enable-demuxer=wav",
        "--disable-parsers",
        "--disable-bsfs",
        "--disable-protocols",
        "--enable-protocol=file",
        "--enable-static",
        "--disable-shared",
        "--disable-autodetect",
        # Disable x86/x86_64 assembly optimisations. Avoids a NASM/YASM dependency
        # and keeps the build reproducible; enable-runtime-cpudetect stays harmless.
        "--disable-asm",
    ] + select({
        # MSVC build; cl.exe comes from the msvc-dev-cmd environment, bash from MSYS.
        "@platforms//os:windows": ["--toolchain=msvc"],
        # NDK cross-compilation; CC/CXX/AR point at NDK clang via rules_foreign_cc's
        # exported toolchain env vars, expanded by the generated configure wrapper.
        ":_android_armv7": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=arm",
            "--cpu=armv7-a",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC) --target=armv7a-linux-androideabi24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        ":_android_arm64": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=aarch64",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC) --target=aarch64-linux-android24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        ":_android_x86_64": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=x86_64",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC) --target=x86_64-linux-android24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        "//conditions:default": [],
    }),
    # Provides the $(CC)/$(AR) make variables used in the android configure options.
    toolchains = ["@bazel_tools//tools/cpp:current_cc_toolchain"],
    out_static_libs = select({
        # MSVC-style library names (LIBPREF=""/LIBSUF=".lib" with --toolchain=msvc).
        "@platforms//os:windows": [
            "avcodec.lib",
            "avformat.lib",
            "avutil.lib",
            "swscale.lib",
            "swresample.lib",
        ],
        "//conditions:default": [
            "libavcodec.a",
            "libavformat.a",
            "libavutil.a",
            "libswscale.a",
            "libswresample.a",
        ],
    }),
    # avformat and avcodec have circular symbol references at runtime initialisation
    # (avformat pulls in avcodec decoders, avcodec calls avformat helpers).
    # alwayslink forces --whole-archive on all five archives so the linker includes
    # every symbol unconditionally, resolving the circular dependency without
    # --start-group/--end-group — the same technique used by the prebuilt POSIX build.
    alwayslink = True,
    linkopts = select({
        "@platforms//os:linux": ["-lm", "-lpthread"],
        # macOS: iconv is needed by some FFmpeg demuxers; CoreFoundation for system codecs.
        # Use -Wl,-framework,Name (single string) to avoid two-entry pair ordering
        # issues in Bazel 8+ linkopts handling.
        "@platforms//os:macos": ["-liconv", "-Wl,-framework,CoreFoundation"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
