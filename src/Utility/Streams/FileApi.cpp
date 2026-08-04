#include "FileApi.h"

#include <cstdio>

#ifdef _WINDOWS
#   define fseeko _fseeki64
#   define ftello _ftelli64
#endif

namespace detail {

// Thin wrappers rather than the libc functions themselves - `fread` and `fwrite` are macros under `_FORTIFY_SOURCE`,
// so their addresses can't be taken, and the seek and tell calls are spelled differently on Windows.

static FILE *openFileImpl(const char *path, const char *mode) {
    return fopen(path, mode);
}

static int closeFileImpl(FILE *file) {
    return fclose(file);
}

static size_t readBytesImpl(void *data, size_t size, size_t count, FILE *file) {
    return fread(data, size, count, file);
}

static size_t writeBytesImpl(const void *data, size_t size, size_t count, FILE *file) {
    return fwrite(data, size, count, file);
}

static int seekImpl(FILE *file, int64_t offset, int origin) {
    return fseeko(file, offset, origin);
}

static int64_t tellImpl(FILE *file) {
    return ftello(file);
}

static int setBufferingImpl(FILE *file, char *buffer, int mode, size_t size) {
    return setvbuf(file, buffer, mode, size);
}

static int flushImpl(FILE *file) {
    return fflush(file);
}

static void clearErrorImpl(FILE *file) {
    clearerr(file);
}

static int checkErrorImpl(FILE *file) {
    return ferror(file);
}

// Both of these are constant-initialized, so `fileApi` is usable from the static initializers of other translation
// units - it doesn't depend on this one having been initialized first.
constinit const FileApi nativeApi = {
    .openFile = &openFileImpl,
    .closeFile = &closeFileImpl,
    .readBytes = &readBytesImpl,
    .writeBytes = &writeBytesImpl,
    .seek = &seekImpl,
    .tell = &tellImpl,
    .setBuffering = &setBufferingImpl,
    .flush = &flushImpl,
    .clearError = &clearErrorImpl,
    .checkError = &checkErrorImpl,
};

constinit const FileApi *fileApi = &nativeApi;

const FileApi *nativeFileApi() {
    return &nativeApi;
}

} // namespace detail
