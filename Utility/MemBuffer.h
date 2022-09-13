#pragma once

#include <memory>

class IMemBuffer {
 public:
    IMemBuffer() {}
    virtual ~IMemBuffer() {}

    virtual const void *GetData() const = 0;
    virtual size_t GetSize() const = 0;
};
typedef std::shared_ptr<IMemBuffer> PMemBuffer;
PMemBuffer AllocMemBuffer(size_t size);
