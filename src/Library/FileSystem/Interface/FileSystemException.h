#pragma once

#include <string>

#include "Utility/Exception.h"

#include "FileSystemEnums.h"
#include "FileSystemFwd.h"

class FileSystemException : public Exception {
 public:
    FileSystemException(FileSystemError error, std::string_view arg0, std::string_view arg1 = {});

    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, const FileSystemPath &arg0);
    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, const FileSystemPath &arg0, const FileSystemPath &arg1);

 private:
    std::string formatMessage(FileSystemError error, std::string_view arg0, std::string_view arg1);
};
