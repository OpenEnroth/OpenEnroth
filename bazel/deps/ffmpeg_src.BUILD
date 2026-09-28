# configure_make() build for FFmpeg n7.0, all platforms. The BCR ffmpeg module
# is linux/macos-only (unconditional pthreads, x86-64 nasm on all non-aarch64).

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

# Local config_settings — //bazel/platforms labels can't be used in injected BUILD files.
config_setting(
    name = "_armv7",
    constraint_values = ["@platforms//cpu:armv7"],
)

config_setting(
    name = "_arm64",
    constraint_values = ["@platforms//cpu:arm64"],
)

config_setting(
    name = "_android_armv7",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:armv7",
    ],
)

config_setting(
    name = "_android_arm64",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "_android_x86_64",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "_android_x86",
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_32",
    ],
)

# Windows -c dbg compiles the engine /MTd; ffmpeg must be on the same debug CRT
# (see the configure_options select).
config_setting(
    name = "_windows_dbg",
    constraint_values = ["@platforms//os:windows"],
    values = {"compilation_mode": "dbg"},
)

filegroup(
    name = "all_srcs",
    srcs = glob(
        ["**"],
        exclude = ["BUILD.bazel"],
    ),
)

# w64: wavdec.c references ff_w64_guid_data behind if(CONFIG_W64_DEMUXER),
# counting on dead-code elimination that MSVC doesn't do here.
_WINDOWS_CONFIGURE_OPTIONS = [
    "--toolchain=msvc",
    "--enable-demuxer=w64",
]

configure_make(
    name = "ffmpeg",
    # .d files are useless in a one-shot bazel build, and the msvc dep pipeline
    # (cl -showIncludes | awk) breaks on backslash mangling in make's shell.
    args = select({
        "@platforms//os:windows": [
            "CCDEP=:",
            "HOSTCCDEP=:",
            # cl under MSYS2 make loses the INCLUDE env var somewhere in the
            # msys runtime hops; make exports command-line variables to recipes.
            "INCLUDE=\"$INCLUDE\"",
        ],
        "//conditions:default": [],
    }),
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
    ] + select({
        # arm asm assembles through the target compiler; x86 asm would need
        # nasm, keep it off (the deps repo also disabled it for android x86).
        ":_arm64": [],
        ":_armv7": [],
        "//conditions:default": ["--disable-asm"],
    }) + select({
        "@platforms//os:windows": _WINDOWS_CONFIGURE_OPTIONS,
        # ffmpeg's msvc toolchain compiles -MD; -MTd comes later on the compile
        # line, so it wins and matches the debug CRT of the rest of the build.
        ":_windows_dbg": _WINDOWS_CONFIGURE_OPTIONS + ["--extra-cflags=-MTd"],
        # NDK cross-compilation via $(CC)/$(AR); per-ABI cpu tuning mirrors the
        # deps repo's build_all.sh.
        ":_android_armv7": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=arm",
            "--cpu=cortex-a8",
            "--enable-neon",
            "--enable-thumb",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC)",
            "--extra-cflags=--target=armv7a-linux-androideabi24",
            "--extra-cflags=-march=armv7-a",
            "--extra-cflags=-mcpu=cortex-a8",
            "--extra-cflags=-mfpu=vfpv3-d16",
            "--extra-cflags=-mfloat-abi=softfp",
            "--extra-cflags=-mthumb",
            "--extra-ldflags=--target=armv7a-linux-androideabi24",
            "--extra-ldflags=-Wl,--fix-cortex-a8",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        ":_android_arm64": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=aarch64",
            "--enable-neon",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC)",
            "--extra-cflags=--target=aarch64-linux-android24",
            "--extra-ldflags=--target=aarch64-linux-android24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        ":_android_x86_64": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=x86_64",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC)",
            "--extra-cflags=--target=x86_64-linux-android24",
            "--extra-cflags=-march=atom",
            "--extra-cflags=-msse3",
            "--extra-cflags=-ffast-math",
            "--extra-cflags=-mfpmath=sse",
            "--extra-ldflags=--target=x86_64-linux-android24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        ":_android_x86": [
            "--enable-cross-compile",
            "--target-os=android",
            "--arch=x86",
            "--cc=$$EXT_BUILD_ROOT$$/$(CC)",
            "--extra-cflags=--target=i686-linux-android24",
            "--extra-cflags=-march=atom",
            "--extra-cflags=-msse3",
            "--extra-cflags=-ffast-math",
            "--extra-cflags=-mfpmath=sse",
            "--extra-ldflags=--target=i686-linux-android24",
            "--ar=$$EXT_BUILD_ROOT$$/$(AR)",
        ],
        "//conditions:default": [],
    }),
    lib_source = ":all_srcs",
    linkopts = select({
        "@platforms//os:linux": [
            "-lm",
            "-lpthread",
        ],
        # av_random_bytes uses BCryptGenRandom. The deps repo patched
        # HAVE_BCRYPT off instead; linking it is simpler here.
        "@platforms//os:windows": ["-DEFAULTLIB:bcrypt.lib"],
        # Single-string -Wl,-framework,Name dodges Bazel 8 linkopt pair reordering.
        "@platforms//os:macos": [
            "-liconv",
            "-Wl,-framework,CoreFoundation",
        ],
        "//conditions:default": [],
    }),
    # ffmpeg names static libs lib*.a on every toolchain, msvc included
    # (lib.exe archives with an .a extension; link.exe consumes them fine).
    out_static_libs = [
        "libavcodec.a",
        "libavformat.a",
        "libavutil.a",
        "libswscale.a",
        "libswresample.a",
    ],
    # Provides the $(CC)/$(AR) make variables used in the android configure options.
    toolchains = ["@bazel_tools//tools/cpp:current_cc_toolchain"],
    visibility = ["//visibility:public"],
    # --whole-archive resolves the avformat<->avcodec circular refs without
    # --start-group. Not on windows: link.exe iterates archives itself, and
    # WHOLEARCHIVE trips LNK2005 on the file_open.o copies in all three libs.
    alwayslink = select({
        "@platforms//os:windows": False,
        "//conditions:default": True,
    }),
)
