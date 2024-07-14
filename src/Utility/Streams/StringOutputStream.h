#pragma once

#include <string>

#include "OutputStream.h"

class StringOutputStream: public OutputStream {
 public:
    StringOutputStream() = default;
    explicit StringOutputStream(std::string *target, std::string_view displayPath = {});
    virtual ~StringOutputStream();

    void open(std::string *target, std::string_view displayPath = {});

    virtual void write(const void *data, size_t size) override;
    virtual void flush() override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

    using OutputStream::write;

 private:
    std::string *_target = nullptr;
    std::string _displayPath;
};
