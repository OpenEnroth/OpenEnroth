#include "StringOutputStream.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <string>

StringOutputStream::StringOutputStream(std::string *target, std::string_view displayPath) {
    open(target, displayPath);
}

StringOutputStream::~StringOutputStream() {
    closeInternal();
}

void StringOutputStream::open(std::string *target, std::string_view displayPath) {
    assert(target);
    _target = target;
    _target->clear();
    base_type::open({}, displayPath);
}

void StringOutputStream::_overflow(const void *data, size_t size, Buffer *buffer) {
    assert(size > buffer->remaining());

    // Geometric growth: 1KB -> 2KB -> ... -> 1MB (cap), but at least enough for the overflow data.
    size_t chunkSize = std::max(size, _target->size() == 0 ? 1024 : std::min<size_t>(_target->size(), 1024 * 1024));

    // Grow the string: overflow data at the start of the new region, remaining space is the new buffer.
    // What we're doing here is technically UB, but we don't really have a choice.
    size_t contentSize = _target->size() - buffer->remaining();
    _target->resize_and_overwrite(_target->size() + chunkSize, [](char *, size_t n) { return n; });
    buffer->reset(_target->data(), _target->data() + contentSize, _target->data() + _target->size());
    buffer->write(data, size);
}

void StringOutputStream::_flush(Buffer *buffer) {
    _target->resize(_target->size() - buffer->remaining());
    buffer->reset(_target->data(), _target->data() + _target->size(), _target->data() + _target->size());
}

void StringOutputStream::_close() {
    assert(isOpen());
    closeInternal();
    base_type::_close();
}

void StringOutputStream::closeInternal() {
    if (!isOpen())
        return;

    _target->resize(_target->size() - buffer().remaining());
    _target = nullptr;
}
