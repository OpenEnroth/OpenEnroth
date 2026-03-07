#pragma once

#include <memory>
#include <string_view>

#include "Utility/Streams/BlobOutputStream.h"

namespace detail {

struct MemoryFileData;

/**
 * Output stream for writing to a memory file system entry. Delegates all I/O to `BlobOutputStream`
 * and manages writer counting on top.
 */
class MemoryFileSystemOutputStream : public BlobOutputStream {
    using base_type = BlobOutputStream;

 public:
    explicit MemoryFileSystemOutputStream(std::shared_ptr<MemoryFileData> data, std::string_view displayPath);
    ~MemoryFileSystemOutputStream();

 private:
    virtual void _close(Buffer *buffer) override;

 private:
    std::shared_ptr<MemoryFileData> _data;
};

} // namespace detail
