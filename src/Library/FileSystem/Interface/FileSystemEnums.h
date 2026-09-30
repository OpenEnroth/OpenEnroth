#pragma once

/**
 * Naming is is `FS_<OP>_FAILED_<REASON>`.
 *
 * Reasons are sorted by priority. It is expected that if an operation fails due to several different problems, then
 * the highest priority `Code` will be used. E.g. if a filesystem isn't writeable, it doesn't matter if we're trying
 * to remove root - the error on `remove` is always that the path is not writeable (because it can't be).
 */
enum class FileSystemError {
    FS_LS_FAILED_PATH_DOESNT_EXIST,
    FS_LS_FAILED_PATH_IS_FILE,
    FS_LS_FAILED_PATH_NOT_ACCESSIBLE, // Escaping path.

    FS_READ_FAILED_PATH_NOT_READABLE,
    FS_READ_FAILED_PATH_DOESNT_EXIST,
    FS_READ_FAILED_PATH_IS_DIR,
    FS_READ_FAILED_PATH_NOT_ACCESSIBLE,

    FS_WRITE_FAILED_PATH_NOT_WRITEABLE, // E.g. on a read-only filesystem.
    FS_WRITE_FAILED_FILE_IN_PATH, // Writing to "a/b.txt/c" where "a/b.txt" is an existing file.
    FS_WRITE_FAILED_PATH_IS_DIR,
    FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE,

    FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, // E.g. is root.
    FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE,
};
using enum FileSystemError;
