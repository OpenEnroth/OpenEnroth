#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace detail {

/**
 * The C file API, as a table of function pointers.
 *
 * `FileInputStream` and `FileOutputStream` go through this table instead of calling libc directly, so that tests can
 * substitute one that fails on demand. There is a single table for the whole process, streams don't carry a pointer
 * to it.
 *
 * Members aren't named after the libc functions they wrap because `fread` and `fwrite` are macros under
 * `_FORTIFY_SOURCE`.
 */
struct FileApi {
    FILE *(*openFile)(const char *path, const char *mode);
    int (*closeFile)(FILE *file);
    size_t (*readBytes)(void *data, size_t size, size_t count, FILE *file);
    size_t (*writeBytes)(const void *data, size_t size, size_t count, FILE *file);
    int (*seek)(FILE *file, int64_t offset, int origin);
    int64_t (*tell)(FILE *file);
    int (*setBuffering)(FILE *file, char *buffer, int mode, size_t size);
    int (*flush)(FILE *file);
    void (*clearError)(FILE *file);
    int (*checkError)(FILE *file);
};

/**
 * @return                              Table that passes everything through to libc.
 */
const FileApi *nativeFileApi();

/**
 * Table that the file streams call through. Never null, defaults to `nativeFileApi()`.
 */
extern const FileApi *fileApi;

/**
 * Scoped override of `fileApi`, for tests that need the streams to see IO errors.
 *
 * The previous table is restored on destruction, so a test that fails partway through can't leave the override in
 * place for the rest of the run. Note that the table is global, so this only works while tests run one at a time.
 */
class ScopedFileApi {
 public:
    explicit ScopedFileApi(const FileApi *api) : _previous(fileApi) {
        fileApi = api;
    }

    ~ScopedFileApi() {
        fileApi = _previous;
    }

    ScopedFileApi(const ScopedFileApi &) = delete;
    ScopedFileApi &operator=(const ScopedFileApi &) = delete;

 private:
    const FileApi *_previous;
};

} // namespace detail
