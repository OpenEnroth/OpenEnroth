#pragma once

#include <cstdio>
#include <string>
#include <string_view>

#include "Utility/Types.h"

#include "InputStream.h"

class FileInputStream : public InputStream {
 public:
    FileInputStream() = default;
    explicit FileInputStream(std::string_view path);
    virtual ~FileInputStream();

    void open(std::string_view path);

    [[nodiscard]] bool isOpen() const {
        return _file != nullptr;
    }

    [[nodiscard]] virtual size_t read(void *data, size_t size) override;
    [[nodiscard]] virtual size_t skip(size_t size) override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

    void seek(ssize_t pos);

    [[nodiscard]] FILE *handle() {
        return _file;
    }

 private:
    void closeInternal(bool canThrow);

 private:
    std::string _path;
    FILE *_file = nullptr;
};
