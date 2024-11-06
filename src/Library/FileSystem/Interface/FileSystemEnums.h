#pragma once

enum class FileType {
    FILE_INVALID, // Returned by `AbstractFileSystem::stat` if file doesn't exist.
    FILE_REGULAR,
    FILE_DIRECTORY,
};
using enum FileType;

/**
 * Naming is is `FS_<OP>_FAILED_<REASON>`.
 *
 * Reasons are sorted by priority. It is expected that if an operation fails due to several different problems, then
 * the highest priority `Code` will be used. E.g. if a filesystem isn't writeable, it doesn't matter if we're trying
 * to remove root - the error on `rename` is always that the target is not writeable (because it can't be).
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

    FS_RENAME_FAILED_DST_NOT_WRITEABLE,
    FS_RENAME_FAILED_SRC_NOT_WRITEABLE, // E.g. is root.
    FS_RENAME_FAILED_DST_NOT_ACCESSIBLE,
    FS_RENAME_FAILED_SRC_NOT_ACCESSIBLE,
    FS_RENAME_FAILED_SRC_DOESNT_EXIST,
    FS_RENAME_FAILED_DST_IS_DIR,
    FS_RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE,
    FS_RENAME_FAILED_SRC_IS_PARENT_OF_DST,

    FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, // E.g. is root.
    FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE,
};
using enum FileSystemError;
