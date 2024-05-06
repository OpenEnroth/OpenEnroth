#pragma once

#include <string>

#include "Utility/Exception.h"

#include "FileSystemFwd.h"

class FileSystemException : public Exception {
 public:
    /**
     * Naming is is `<OP>_FAILED_<REASON>`.
     *
     * Reasons are sorted by priority. It is expected that if an operation fails due to several different problems, then
     * the highest priority `Code` will be used. E.g. if a filesystem isn't writeable, it doesn't matter if we're trying
     * to remove root - the error on `rename` is always that the target is not writeable (because it can't be).
     */
    enum class Code {
        LS_FAILED_PATH_DOESNT_EXIST,
        LS_FAILED_PATH_IS_FILE,

        READ_FAILED_PATH_NOT_READABLE,
        READ_FAILED_PATH_DOESNT_EXIST,
        READ_FAILED_PATH_IS_DIR,

        WRITE_FAILED_PATH_NOT_WRITEABLE, // E.g. on a read-only filesystem.
        WRITE_FAILED_PATH_IS_DIR,

        RENAME_FAILED_DST_NOT_WRITEABLE,
        RENAME_FAILED_SRC_NOT_WRITEABLE, // E.g. is root.
        RENAME_FAILED_SRC_DOESNT_EXIST,
        RENAME_FAILED_DST_IS_DIR,
        RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE,
        RENAME_FAILED_SRC_IS_PARENT_OF_DST,

        REMOVE_FAILED_PATH_NOT_WRITEABLE, // E.g. is root.
    };
    using enum Code;

    FileSystemException(Code code, const FileSystemPath &arg0);
    FileSystemException(Code code, const FileSystemPath &arg0, const FileSystemPath &arg1);

 private:
    std::string formatMessage(Code code, const FileSystemPath &arg0, const FileSystemPath &arg1);
};
