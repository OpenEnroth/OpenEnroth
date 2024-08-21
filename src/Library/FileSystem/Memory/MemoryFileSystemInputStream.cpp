#include "MemoryFileSystemInputStream.h"

#include <cassert>
#include <memory>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemInputStream::MemoryFileSystemInputStream(std::shared_ptr<MemoryFileData> data) {
    assert(data);
    assert(data->writerCount == 0);

    _data = std::move(data);
    _data->readerCount++;
    open(_data->blob);
}

MemoryFileSystemInputStream::~MemoryFileSystemInputStream() {
    closeInternal();
}

void MemoryFileSystemInputStream::close() {
    BlobInputStream::close();
    closeInternal();
}

void MemoryFileSystemInputStream::closeInternal() {
    if (!_data)
        return;

    _data->readerCount--;
    _data.reset();
}

} // namespace detail
