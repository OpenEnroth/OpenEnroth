#pragma once

#include <string>

#include "FileOutputStream.h"

// TODO(captainurist): unfortunately this design is flawed. You need an explicit commit() for this to work in a sane
//                     way. Otherwise the enclosing code throws an exception, and then we just overwrite the target
//                     file in the destructor, which is obviously not what we want to do here.
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
