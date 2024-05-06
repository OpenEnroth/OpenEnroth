#pragma once

#include <memory>

#include "Library/FileSystem/Trie/FileSystemTrie.h"

#include "Utility/Streams/BlobInputStream.h"

namespace detail {

struct MemoryFileData;

class MemoryFileSystemInputStream : public BlobInputStream {
 public:
    explicit MemoryFileSystemInputStream(std::shared_ptr<MemoryFileData> data);
    virtual ~MemoryFileSystemInputStream();

 private:
    virtual void close() override;
    void closeInternal();

 private:
    std::shared_ptr<MemoryFileData> _data;
};

} // namespace detail
