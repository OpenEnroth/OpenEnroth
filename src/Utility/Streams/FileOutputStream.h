#pragma once

#include <cstdio>
#include <string>

#include "OutputStream.h"

class FileOutputStream : public OutputStream {
 public:
    explicit FileOutputStream(const std::string &path);
    virtual ~FileOutputStream();
    virtual void Write(const void *data, size_t size) override;
    virtual void Flush() override;
    virtual void Close() override;

 private:
    void CloseInternal(bool canThrow);
    [[noreturn]] void ThrowFromErrno();

 private:
    std::string path_;
    FILE *file_ = nullptr;
};
