#include "Media/Media.h"

#include <memory>

class MemBuffer : public IMemBuffer {
 public:
  MemBuffer(size_t size) : size(size) { data = malloc(size); }
  virtual ~MemBuffer() { delete data; }

  virtual const void *get_data() const { return data; }
  virtual size_t get_size() const { return size; }

  virtual void *get_writable_data() const { return data; }

 public:
  void *data;
  size_t size;
};

PMemBuffer AllocMemBuffer(size_t size) {
  std::shared_ptr<MemBuffer> buffer = std::make_shared<MemBuffer>(size);
  return std::reinterpret_pointer_cast<IMemBuffer, MemBuffer>(buffer);
}
