#!/bin/bash
# Builds gcc-15 from source and installs to the path passed as $1.
#
# Why? ubuntu-24.04 (the runner OS we use) ships gcc-13, which is too old for the C++23 features
# OpenEnroth uses. The only apt source for gcc-15 on noble is the ubuntu-toolchain-r/test PPA, which
# is a free, single-region (UK), uncached, per-IP-throttled launchpad service. CI installs from
# that PPA frequently stall or fail (often <10 KB/s effective throughput, occasional connection
# refusals). Building gcc-15 once and caching the install directory is more reliable than depending
# on the PPA every run.
#
# Usage: scripts/build_gcc_15.sh <prefix>
#
# Produces <prefix>/{bin,lib,...} with binaries named gcc-15 / g++-15 (matching the existing
# CMAKE_C_COMPILER / CMAKE_CXX_COMPILER values in build_all.yml).
#
# `--enable-multilib` so the same install serves both x86_64 and x86 (32-bit) targets.
# `--disable-bootstrap` keeps the build to a single stage (~30 min instead of ~90 min for a full
# 3-stage bootstrap; we don't need a self-hosting compiler in CI).
set -euxo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <prefix>" >&2
    exit 1
fi

PREFIX="$1"
GCC_VERSION="${GCC_VERSION:-15.2.0}"

WORKDIR="$(mktemp -d)"
cd "${WORKDIR}"
curl -sSLO "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
tar xf "gcc-${GCC_VERSION}.tar.xz"

mkdir build
cd build
"../gcc-${GCC_VERSION}/configure" \
    --prefix="${PREFIX}" \
    --enable-languages=c,c++ \
    --enable-multilib \
    --disable-bootstrap \
    --disable-libsanitizer \
    --disable-nls \
    --without-included-gettext \
    --program-suffix=-15

make -j"$(nproc)"
make install

# Free disk - the source tree and build dir are huge (~5 GB) and we're done with them.
rm -rf "${WORKDIR}"
