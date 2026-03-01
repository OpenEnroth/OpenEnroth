#include "MemoryFileSystemOutputStream.h"

#include <cassert>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemOutputStream::MemoryFileSystemOutputStream(std::shared_ptr<MemoryFileData> data, std::string_view displayPath) : _data(std::move(data)) {
    assert(_data);
    assert(_data->readerCount == 0);
    assert(_data->writerCount == 0);

    _data->writerCount++;
    BlobOutputStream::open(&_data->blob, displayPath);
}

MemoryFileSystemOutputStream::~MemoryFileSystemOutputStream() {
    closeInternal();
}

void MemoryFileSystemOutputStream::_close() {
    closeInternal();
    BlobOutputStream::_close();
}

void MemoryFileSystemOutputStream::closeInternal() {
    if (!_data)
        return;
    _data->writerCount--;
    _data.reset();
}

} // namespace detail
