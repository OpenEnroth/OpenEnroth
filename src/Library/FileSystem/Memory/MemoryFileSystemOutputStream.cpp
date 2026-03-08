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
    base_type::open(&_data->blob, displayPath);
}

MemoryFileSystemOutputStream::~MemoryFileSystemOutputStream() {
    // Must close here because members are destroyed before the base class destructor runs, and the base class
    // destructor flushes buffered data into `_data->blob`. Without this call `_data` would be dead by then.
    close();
}

void MemoryFileSystemOutputStream::_close(Buffer *buffer) {
    // Must call base_type::_close() before releasing `_data` — it flushes buffered data into `_data->blob`.
    base_type::_close(buffer);
    _data->writerCount--;
    _data.reset();
}

} // namespace detail
