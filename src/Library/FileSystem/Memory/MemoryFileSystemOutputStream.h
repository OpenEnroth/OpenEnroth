#pragma once

#include <memory>

#include "Library/FileSystem/Trie/FileSystemTrie.h"

#include "Utility/Streams/BlobOutputStream.h"

namespace detail {

struct MemoryFileData;

class MemoryFileSystemOutputStream : public BlobOutputStream {
 public:
    explicit MemoryFileSystemOutputStream(std::shared_ptr<MemoryFileData> data, std::string_view displayPath);
    virtual ~MemoryFileSystemOutputStream();

 private:
    virtual void close() override;
    void closeInternal();

 private:
    std::shared_ptr<MemoryFileData> _data;
};

} // namespace detail
