# configure_make() build for FFmpeg n7.0 from source.
# Replaces the prebuilt FFmpeg from OpenEnroth_Dependencies on Linux and macOS.
# Windows is kept on prebuilt (FFmpeg configure requires bash/MSYS2, deferred).

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"], exclude = ["BUILD.bazel"]),
)

configure_make(
    name = "ffmpeg",
    lib_source = ":all_srcs",
    configure_options = [
        "--disable-programs",   # Don't build ffmpeg/ffprobe/ffplay executables.
        "--disable-doc",        # No documentation.
        "--disable-htmlpages",
        "--disable-manpages",
        "--disable-podpages",
        "--disable-txtpages",
        "--enable-static",      # Build static libraries.
        "--disable-shared",     # No shared libraries.
        # Don't auto-detect optional external libraries (libmp3lame, x264, etc.).
        # Keeps the build hermetic — only built-in codec support is compiled.
        "--disable-autodetect",
        # Disable x86/x86_64 assembly optimisations.
        # Avoids a NASM/YASM dependency and keeps the build reproducible across
        # environments that may not have an assembler available.
        "--disable-asm",
    ],
    out_static_libs = [
        "libavcodec.a",
        "libavformat.a",
        "libavutil.a",
        "libswscale.a",
        "libswresample.a",
    ],
    # avformat and avcodec have circular symbol references at runtime initialisation
    # (avformat pulls in avcodec decoders, avcodec calls avformat helpers).
    # alwayslink forces --whole-archive on all five archives so the linker includes
    # every symbol unconditionally, resolving the circular dependency without
    # --start-group/--end-group — the same technique used by the prebuilt POSIX build.
    alwayslink = True,
    linkopts = select({
        "@platforms//os:linux": ["-lm", "-lpthread"],
        # macOS: iconv is needed by some FFmpeg demuxers; CoreFoundation for system codecs.
        "@platforms//os:macos": ["-liconv", "-framework CoreFoundation"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
