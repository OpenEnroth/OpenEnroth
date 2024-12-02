#include "SequentialBlobReader.h"

#include <cassert>

#include <string>

SequentialBlobReader::SequentialBlobReader(const uint8_t *data, size_t size) : dataRemaining(data), dataEnd(data + size) {}

std::string SequentialBlobReader::readString() {
    assert(dataRemaining < dataEnd);

    const uint8_t* pos = static_cast<const uint8_t*>(memchr(dataRemaining, 0, dataEnd - dataRemaining));
    assert(pos != nullptr && "please report");
    size_t length = pos - dataRemaining;
    std::string val = std::string(reinterpret_cast<const char*>(dataRemaining), length);
    dataRemaining += length + 1;  // consume also the NUL

    return val;
}
