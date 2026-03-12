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
    base_type::open(_data->blob);
}

MemoryFileSystemInputStream::~MemoryFileSystemInputStream() {
    destroy();
}

void MemoryFileSystemInputStream::_close(bool canThrow) {
    assert(_data);
    _data->readerCount--;
    _data.reset();
    base_type::_close(canThrow);
}

} // namespace detail
