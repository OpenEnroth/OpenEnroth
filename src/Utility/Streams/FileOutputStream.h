#pragma once

#include <cstdio>
#include <string>
#include <string_view>

#include "OutputStream.h"

class FileOutputStream : public OutputStream {
 public:
    explicit FileOutputStream(std::string_view path);
    virtual ~FileOutputStream();
    virtual void Write(const void *data, size_t size) override;
    virtual void Flush() override;
    virtual void Close() override;

 private:
    void CloseInternal(bool canThrow);

 private:
    std::string path_;
    FILE *file_ = nullptr;
};
