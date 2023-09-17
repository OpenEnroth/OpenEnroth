#pragma once

#include <string>

#include "OutputStream.h"

class StringOutputStream: public OutputStream {
 public:
    explicit StringOutputStream(std::string *target);
    virtual ~StringOutputStream();
    virtual void write(const void *data, size_t size) override;
    virtual void flush() override;
    virtual void close() override;

    using OutputStream::write;

 private:
    std::string *_target = nullptr;
};
