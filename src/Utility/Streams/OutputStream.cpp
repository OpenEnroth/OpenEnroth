#include "OutputStream.h"

#include <cassert>

OutputStream::~OutputStream() = default;

void OutputStream::_close() {
    assert(isOpen());
    closeInternal();
}

void OutputStream::open(Buffer buffer, std::string_view displayPath) {
    _buffer = buffer;
    _isOpen = true;
    _displayPath = displayPath;
}

void OutputStream::closeInternal() {
    _buffer.reset(nullptr, nullptr, nullptr);
    _isOpen = false;
    _displayPath = {};
}

