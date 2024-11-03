#include "MemoryFileSystemOutputStream.h"

#include <memory>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemOutputStream::MemoryFileSystemOutputStream(std::shared_ptr<MemoryFileData> data, std::string_view displayPath) {
    assert(data);
    assert(data->readerCount == 0);
    assert(data->writerCount == 0);

    _data = std::move(data);
    _data->writerCount++;
    open(&_data->blob, displayPath);
}

MemoryFileSystemOutputStream::~MemoryFileSystemOutputStream() {
    // We can't just call closeInternal() here because base class's close() depends on _data->blob.
    close();
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
