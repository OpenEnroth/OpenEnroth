#pragma once

#include <cstdio>
#include <string>
#include <string_view>

#include "InputStream.h"

class FileInputStream : public InputStream {
 public:
    FileInputStream() = default;
    explicit FileInputStream(std::string_view path);
    virtual ~FileInputStream();

    void Open(std::string_view path);

    bool IsOpen() const {
        return _file != nullptr;
    }

    virtual size_t Read(void *data, size_t size) override;
    virtual size_t Skip(size_t size) override;
    virtual void Close() override;
    void Seek(size_t pos);

 private:
    void CloseInternal(bool canThrow);

 private:
    std::string _path;
    FILE *_file = nullptr;
};
