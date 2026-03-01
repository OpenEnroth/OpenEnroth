#include "MemoryFileSystemInputStream.h"

#include <cassert>
#include <memory>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemInputStream::MemoryFileSystemInputStream(std::shared_ptr<MemoryFileData> data) : _data(std::move(data)) {
    assert(_data);
    assert(_data->writerCount == 0);

    _data->readerCount++;
    const char *start = static_cast<const char *>(_data->blob.data());
    InputStream::open(start, start + _data->blob.size(), _data->blob.displayPath());
}

MemoryFileSystemInputStream::~MemoryFileSystemInputStream() {
    closeInternal();
}

void MemoryFileSystemInputStream::_close() {
    closeInternal();
    InputStream::_close();
}

void MemoryFileSystemInputStream::closeInternal() {
    if (!_data)
        return;
    _data->readerCount--;
    _data.reset();
}

} // namespace detail
