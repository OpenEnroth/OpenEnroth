#pragma once

#include <string>

#include "Utility/Exception.h"

#include "FileSystemFwd.h"
#include "FileSystemEnums.h"
#include "FileSystemPathView.h"

class FileSystemException : public Exception {
 public:
    FileSystemException(FileSystemError error, std::string_view arg0);

    [[noreturn]] static void raise(const FileSystem *fs, FileSystemError error, FileSystemPathView arg0);

 private:
    std::string formatMessage(FileSystemError error, std::string_view arg0);
};
