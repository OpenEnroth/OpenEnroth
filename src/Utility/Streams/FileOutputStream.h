#pragma once

#include <cstdio>
#include <string>
#include <string_view>

#include "OutputStream.h"

class FileOutputStream : public OutputStream {
 public:
    FileOutputStream() = default;
    explicit FileOutputStream(std::string_view path);
    virtual ~FileOutputStream();

    void Open(std::string_view path);

    bool IsOpen() const {
        return _file != nullptr;
    }

    virtual void Write(const void *data, size_t size) override;
    virtual void Flush() override;
    virtual void Close() override;

 private:
    void CloseInternal(bool canThrow);

 private:
    std::string _path;
    FILE *_file = nullptr;
};
