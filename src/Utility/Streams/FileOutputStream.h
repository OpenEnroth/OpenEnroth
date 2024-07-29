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

    void open(std::string_view path);

    [[nodiscard]] bool isOpen() const {
        return _file != nullptr;
    }

    virtual void write(const void *data, size_t size) override;
    using OutputStream::write;
    virtual void flush() override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

    [[nodiscard]] FILE *handle() {
        return _file;
    }

 private:
    void closeInternal(bool canThrow);

 private:
    std::string _path;
    FILE *_file = nullptr;
};
