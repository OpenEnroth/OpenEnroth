#include "MemBuffer.h"

#include <memory>

class MemBuffer : public IMemBuffer {
 public:
    explicit MemBuffer(size_t size) : size(size) { data = malloc(size); }
    virtual ~MemBuffer() { free(data); }

    virtual const void *GetData() const { return data; }
    virtual size_t GetSize() const { return size; }

    virtual void *GetWritableData() const { return data; }

 public:
    void *data;
    size_t size;
};

PMemBuffer AllocMemBuffer(size_t size) {
    std::shared_ptr<MemBuffer> buffer = std::make_shared<MemBuffer>(size);
    return std::dynamic_pointer_cast<IMemBuffer, MemBuffer>(buffer);
}
