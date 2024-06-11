#include "MemoryFileSystemOutputStream.h"

#include <memory>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemOutputStream::MemoryFileSystemOutputStream(std::shared_ptr<MemoryFileData> data) {
    assert(data);
    assert(data->blob.empty());
    assert(data->readerCount == 0);
    assert(data->writerCount == 0);

    _data = std::move(data);
    _data->writerCount++;
    open(&_data->blob);
}

MemoryFileSystemOutputStream::~MemoryFileSystemOutputStream() {
    closeInternal();
}

void MemoryFileSystemOutputStream::close() {
    BlobOutputStream::close();
    closeInternal();
}

void MemoryFileSystemOutputStream::closeInternal() {
    if (!_data)
        return;

    _data->writerCount--;
    _data.reset();
}

} // namespace detail
