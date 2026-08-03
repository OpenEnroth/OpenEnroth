# configure_make() build for FFmpeg n7.0 from source.
# Used on the 32-bit targets that the BCR ffmpeg module can't serve: linux_x86,
# windows_x86 and android_armeabi_v7a. 64-bit platforms use @ffmpeg from the BCR.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

# Local config_settings — //bazel/platforms labels can't be used in injected BUILD files.
config_setting(
    name = "_android_armv7",
    constraint_values = ["@platforms//os:android", "@platforms//cpu:armv7"],
)

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
            "--cc=$CC",
            "--cxx=$CXX",
            "--ar=$AR",
        ],
        "//conditions:default": [],
    }),
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
    # After install, remove half2float.o from libswscale.a to eliminate the duplicate
    # _ff_init_half2float_tables symbol. Both libavcodec and libswscale compile half2float.c
    # independently. With alwayslink=True (--whole-archive / -force_load), both copies
    # land in the final binary, causing a duplicate symbol error in lld and Apple ld.
    # Removing it from libswscale is safe: the binary still includes libavcodec.a with
    # the symbol, satisfying any swscale references at final link time.
    postfix_script = "ar d $INSTALLDIR/lib/libswscale.a half2float.o || true",
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
