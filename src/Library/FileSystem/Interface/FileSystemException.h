#pragma once

#include <string>

#include "Utility/Exception.h"

#include "FileSystemFwd.h"
#include "FileSystemEnums.h"
#include "NormalPath.h"

class FileSystemException : public Exception {
 public:
    /**
     * @param error                     What failed.
     * @param displayPath               Path to show in the message, as `FileSystem::displayPath` gives it.
     */
    FileSystemException(FileSystemError error, std::string_view displayPath);

    /**
     * @param fs                        File system that failed.
     * @param error                     What failed.
     * @param path                      Path inside `fs` that failed, which may be inaccessible.
     */
    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, NormalPathView path);

 private:
    std::string formatMessage(FileSystemError error, std::string_view displayPath);
};
