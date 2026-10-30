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

    /**
     * @param fs                        File system that refused the path.
     * @param error                     What failed.
     * @param path                      Path as the caller passed it to `fs`, which may be inaccessible or empty.
     */
    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, PathView path);

 private:
    std::string formatMessage(FileSystemError error, std::string_view arg0);
};
