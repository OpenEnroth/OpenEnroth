#include "StringOutputStream.h"

#include <cassert>
#include <cstring>

StringOutputStream::StringOutputStream(std::string *target) : target_(target) {
    assert(target);
}

StringOutputStream::~StringOutputStream() {}

void StringOutputStream::write(const void *data, size_t size) {
    assert(target_);

    target_->resize(target_->size() + size);
    memcpy(target_->data() + target_->size() - size, data, size);
}

void StringOutputStream::flush() {
    assert(target_);

    // Nothing else to do here.
}

void StringOutputStream::close() {
    target_ = nullptr;
}
