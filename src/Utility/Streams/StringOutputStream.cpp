#include "StringOutputStream.h"

#include <cassert>
#include <cstring>
#include <string>

StringOutputStream::StringOutputStream(std::string *target) : _target(target) {
    assert(target);
}

StringOutputStream::~StringOutputStream() {}

void StringOutputStream::write(const void *data, size_t size) {
    assert(_target);

    _target->resize(_target->size() + size);
    memcpy(_target->data() + _target->size() - size, data, size);
}

void StringOutputStream::flush() {
    assert(_target);

    // Nothing else to do here.
}

void StringOutputStream::close() {
    _target = nullptr;
}
