#pragma once

#include <string>

#include "Utility/Exception.h"
#include "Utility/System/PathView.h"

#include "FileSystemFwd.h"
#include "FileSystemEnums.h"
#include "NormalPath.h"

class FileSystemException : public Exception {
 public:
    FileSystemException(FileSystemError error, std::string_view arg0);

    /**
     * @param fs                        File system that failed.
     * @param error                     What failed.
     * @param path                      Path inside `fs` that the implementation was working with.
     */
    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, NormalPathView path);

 private:
    friend class FileSystem;

    /**
     * @param fs                        File system that refused the path.
     * @param error                     What failed.
     * @param path                      Path that a public method of `fs` refused, which may be inaccessible. An
     *                                  empty path is shown as `''`.
     */
    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, PathView path);

    std::string formatMessage(FileSystemError error, std::string_view arg0);
};
