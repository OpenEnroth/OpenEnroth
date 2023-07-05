#!/bin/bash

TOOLCHAIN_ARCH=linux-x86_64
ALL_ARCHS="armeabi-v7a arm64-v8a x86 x86_64"
FF_VER="4.4.3"

set -e

# NDK example path ~/Android/Sdk/ndk/25.1.8937393

if [[ "${ANDROID_NDK}" == "" ]]; then
    echo "Please provide path to your NDK via NDK environment variable!"
    echo "Example: ANDROID_NDK=~/Android/Sdk/ndk/25.1.8937393 $0"
    exit -1
fi

COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-everything"

# Licensing options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-gpl"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-version3"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-nonfree"

# Configuration options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-static"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-shared"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-small"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-runtime-cpudetect"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-gray"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-swscale-alpha"

# Program options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-programs"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-ffmpeg"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-ffplay"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-ffprobe"

# External library support:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-iconv"

# Documentation options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-doc"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-htmlpages"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-manpages"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-podpages"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-txtpages"

# Component options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-avdevice"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-avcodec"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-avformat"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-avutil"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-swresample"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-swscale"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-postproc"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-avfilter"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-dct"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-mdct"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-rdft"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-fft"

# Individual component options:
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-devices"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-encoders"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-filters"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-hwaccels"

# ./configure --list-decoders
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-decoders"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=mp3*"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=adpcm*"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=pcm*"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=bink"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=binkaudio_dct"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=binkaudio_rdft"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=smackaud"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=smacker"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-decoder=wav"

# ./configure --list-muxers
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-muxers"

# ./configure --list-demuxers
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-demuxers"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=mp3"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=bink"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=binka"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=smacker"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=pcm*"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=wav*"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-demuxer=vorbisaaaaa"

# ./configure --list-parsers
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-parsers"

# ./configure --list-bsf
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-bsfs"

# ./configure --list-protocols
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --disable-protocols"
COMMON_FF_CFG_FLAGS="${COMMON_FF_CFG_FLAGS} --enable-protocol=file"

function build_ffmpeg
{
    FFMPEG_DIR="ffmpeg-${FF_VER}_${ARCH_PREFIX}"

    if [[ -d ${FFMPEG_DIR} ]]; then
        rm -rf ${FFMPEG_DIR}
    fi

    echo "[+++] unpacking ffmpeg v${FF_VER}:${ARCH_PREFIX}..."
    mkdir ${FFMPEG_DIR}
    tar jxf ffmpeg-${FF_VER}.tar.bz2 --strip-components=1 -C ${FFMPEG_DIR}
    pushd ${FFMPEG_DIR}
    patch -p1 < ../ffmpeg-so.patch

    echo "[+++] configuring ffmpeg v${FF_VER}:${ARCH_PREFIX}..."

    CROSS_PREFIX="${ARCH_PREFIX}"
    # at was working at least on ndk21 and was screwed in 25
    if [[ "${TOOLCHAIN_NAME}" == "llvm" ]]; then
        CROSS_PREFIX="llvm"
    fi

    ./configure \
        --prefix=${PREFIX} \
        --disable-symver \
        --cross-prefix=${TOOLCHAIN}/bin/llvm- \
        --cc=${TOOLCHAIN}/bin/${CC} \
        --cxx=${TOOLCHAIN}/bin/${CXX} \
        --target-os=linux \
        --pkg-config=pkg-config \
        --enable-cross-compile \
        --sysroot=${SYSROOT} \
        --extra-cflags="-Os ${EXTRA_CFLAGS}" \
        --extra-ldflags="${EXTRA_LDFLAGS}" \
        ${FF_CFG_FLAGS} \
        ${COMMON_FF_CFG_FLAGS}

    echo "[+++] compilling ffmpeg v${FF_VER}:${ARCH_PREFIX}..."
    make -j$(nproc)
    echo "[+++] installing ffmpeg v${FF_VER}:${ARCH_PREFIX}..."
    make install

    popd

    rm -rf ${FFMPEG_DIR}
}

if [[ ! -e ffmpeg-${FF_VER}.tar.bz2 ]]; then
    echo "[+++] downloading ffmpeg v${FF_VER}..."
    wget http://ffmpeg.org/releases/ffmpeg-${FF_VER}.tar.bz2
fi

CFLAGS_ARMV7A=""
CFLAGS_X86=""

FF_EXTRA_CFLAGS=""
FF_EXTRA_LDFLAGS=""

for ARCH in ${ALL_ARCHS}
do
    FF_CFG_FLAGS=""

    if [ "${ARCH}" = "armeabi-v7a" ]; then
        ANDROID_PLATFORM=18
        ANDROID_PLATFORM_VARIANT=arm
        ARCH_PREFIX=arm-linux-androideabi
        GCC_VER=4.9
        TOOLCHAIN_NAME=${ARCH_PREFIX}-${GCC_VER}

        if [[ ! -d ${ANDROID_NDK}/toolchains/${TOOLCHAIN_NAME}/prebuilt/${TOOLCHAIN_ARCH} ]]; then
            ARCH_PREFIX=armv7a-linux-androideabi
            ANDROID_PLATFORM=21
            TOOLCHAIN_NAME=llvm
        fi

        FF_CFG_FLAGS="${FF_CFG_FLAGS} --arch=arm --cpu=cortex-a8"
        FF_CFG_FLAGS="${FF_CFG_FLAGS} --enable-neon"
        FF_CFG_FLAGS="${FF_CFG_FLAGS} --enable-thumb"

        EXTRA_CFLAGS="${FF_EXTRA_CFLAGS} -march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"
        EXTRA_LDFLAGS="${FF_EXTRA_LDFLAGS} -Wl,--fix-cortex-a8"

        CFLAGS_ARMV7A="${EXTRA_CFLAGS}"
    elif [ "${ARCH}" = "arm64-v8a" ]; then
        ANDROID_PLATFORM=21
        ANDROID_PLATFORM_VARIANT=arm64
        ARCH_PREFIX=aarch64-linux-android
        GCC_VER=4.9
        TOOLCHAIN_NAME=${ARCH_PREFIX}-${GCC_VER}
        if [[ ! -d ${ANDROID_NDK}/toolchains/${TOOLCHAIN_NAME}/prebuilt/${TOOLCHAIN_ARCH} ]]; then
            TOOLCHAIN_NAME=llvm
        fi

        FF_CFG_FLAGS="${FF_CFG_FLAGS} --arch=aarch64"
        FF_CFG_FLAGS="${FF_CFG_FLAGS} --enable-neon"

        EXTRA_CFLAGS="${FF_EXTRA_CFLAGS}"
        EXTRA_LDFLAGS="${FF_EXTRA_LDFLAGS}"
    elif [ "${ARCH}" = "x86" ]; then
        ANDROID_PLATFORM=18
        ANDROID_PLATFORM_VARIANT=x86
        ARCH_PREFIX=i686-linux-android
        GCC_VER=4.9
        TOOLCHAIN_NAME=x86-${GCC_VER}
        if [[ ! -d ${ANDROID_NDK}/toolchains/${TOOLCHAIN_NAME}/prebuilt/${TOOLCHAIN_ARCH} ]]; then
            ANDROID_PLATFORM=21
            TOOLCHAIN_NAME=llvm
            echo "[!!!] WARNING! compiling without asm code, use old NDK with gcc compiler if you want more optimized build!"
            FF_CFG_FLAGS="${FF_CFG_FLAGS} --disable-asm"
        else
            FF_CFG_FLAGS="${FF_CFG_FLAGS} --enable-x86asm --x86asmexe=yasm"
        fi

        FF_CFG_FLAGS="${FF_CFG_FLAGS} --arch=x86 --cpu=i686"

        EXTRA_CFLAGS="${FF_EXTRA_CFLAGS} -march=atom -msse3 -ffast-math -mfpmath=sse"
        EXTRA_LDFLAGS="${FF_EXTRA_LDFLAGS}"

        CFLAGS_X86="${EXTRA_CFLAGS}"
    elif [ "${ARCH}" = "x86_64" ]; then
        ANDROID_PLATFORM=21
        ANDROID_PLATFORM_VARIANT=x86_64
        ARCH_PREFIX=x86_64-linux-android
        GCC_VER=4.9
        TOOLCHAIN_NAME=x86_64-${GCC_VER}
        if [[ ! -d ${ANDROID_NDK}/toolchains/${TOOLCHAIN_NAME}/prebuilt/${TOOLCHAIN_ARCH} ]]; then
            TOOLCHAIN_NAME=llvm
        fi

        FF_CFG_FLAGS="${FF_CFG_FLAGS} --arch=x86_64 --cpu=sse4 --enable-x86asm"

        EXTRA_CFLAGS="${FF_EXTRA_CFLAGS} -march=atom -msse3 -ffast-math -mfpmath=sse"
        EXTRA_LDFLAGS="${FF_EXTRA_LDFLAGS}"

        CFLAGS_X86="${EXTRA_CFLAGS}"
    else
        echo "[!!!] unknown target!"
        exit -1
    fi

    PREFIX=$(pwd)/android/${ARCH}
    TOOLCHAIN=${ANDROID_NDK}/toolchains/${TOOLCHAIN_NAME}/prebuilt/${TOOLCHAIN_ARCH}
    PATH=${ANDROID_NDK}:${PATH}

    if [[ "${TOOLCHAIN_NAME}" == "llvm" ]]; then
        echo "[+++] LLVM Clang compiler for ${ARCH} configured!"
        CC="${ARCH_PREFIX}${ANDROID_PLATFORM}-clang"
        CXX="${ARCH_PREFIX}${ANDROID_PLATFORM}-clang++"
        SYSROOT=${TOOLCHAIN}/sysroot/
    else
        echo "[+++] GNU GCC compiler for ${ARCH} configured!"
        CC="${ARCH_PREFIX}-gcc"
        CXX="${ARCH_PREFIX}-g++"
        SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_PLATFORM}/arch-${ANDROID_PLATFORM_VARIANT}/
    fi


    echo "[+++] building ffmpeg-${ARCH}..."
    build_ffmpeg
done
