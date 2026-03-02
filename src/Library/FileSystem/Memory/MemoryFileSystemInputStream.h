#pragma once

#include <memory>

#include "Library/FileSystem/Trie/FileSystemTrie.h"

#include "Utility/Streams/BlobInputStream.h"

namespace detail {

struct MemoryFileData;

/**
 * Input stream for reading from a memory file system entry. Manages reader counting for the memory file data.
 */
class MemoryFileSystemInputStream : public BlobInputStream {
    using base_type = BlobInputStream;

 public:
    explicit MemoryFileSystemInputStream(std::shared_ptr<MemoryFileData> data);
    ~MemoryFileSystemInputStream();

 private:
    virtual void _close() override;
    void closeInternal();

 private:
    std::shared_ptr<MemoryFileData> _data;
};

} // namespace detail
