#pragma once

/**
 * Naming is `FS_<OP>_FAILED_<REASON>`. `*_PATH_NOT_ACCESSIBLE` means that `NormalPathView::isAccessible` refused the
 * path. `FileSystem` checks accessibility and the root first, then the implementation reports what it finds.
 */
enum class FileSystemError {
    FS_LS_FAILED_PATH_DOESNT_EXIST,
    FS_LS_FAILED_PATH_IS_FILE,
    FS_LS_FAILED_PATH_NOT_ACCESSIBLE,

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
