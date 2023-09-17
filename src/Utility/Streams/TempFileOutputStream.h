#pragma once

#include <string>

#include "FileOutputStream.h"

/**
 * Same as `FileOutputStream`, but writing into a temporary file & moving it in place on close.
 */
class TempFileOutputStream : public FileOutputStream {
    using base_type = FileOutputStream;
 public:
    explicit TempFileOutputStream(std::string_view path);
    virtual ~TempFileOutputStream();

    void open(std::string_view path);

    virtual void close() override;

 private:
    void closeInternal();

 private:
    std::string _targetPath;
    std::string _tmpPath;
};
