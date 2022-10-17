#pragma once

#include <string>

#include "OutputStream.h"

class StringOutputStream: public OutputStream {
public:
    StringOutputStream(std::string *target);
    virtual ~StringOutputStream();
    virtual void Write(const void *data, size_t size) override;
    virtual void Flush() override;
    virtual void Close() override;

private:
    std::string *target_ = nullptr;
};
