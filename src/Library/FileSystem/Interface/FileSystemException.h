#pragma once

#include <string>

#include "Utility/Exception.h"

#include "FileSystemFwd.h"
#include "FileSystemEnums.h"
#include "Utility/System/PathView.h"

class FileSystemException : public Exception {
 public:
    FileSystemException(FileSystemError error, std::string_view arg0);

    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, PathView arg0);

 private:
    std::string formatMessage(FileSystemError error, std::string_view arg0);
};
