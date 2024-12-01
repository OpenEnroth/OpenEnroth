#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <string>

#include "Unaligned.h"

// TODO(yoctozepto, captainurist): consider refactoring due to Utility/Streams/InputStream having a similar abstraction
class SequentialBlobReader {
 public:
    SequentialBlobReader(const uint8_t *data, size_t size);
    template<class T>
    T read() {
        assert(dataRemaining + sizeof(T) <= dataEnd);
        T val;
        if (sizeof(T) == 1) {
            val = T(*dataRemaining);
        } else {
            val = readUnaligned<T>(dataRemaining);
        }
        dataRemaining += sizeof(T);
        return val;
    }
    std::string readString();
    inline bool readable() {
        return dataRemaining < dataEnd;
    }

 private:
    const uint8_t *const dataEnd;
    const uint8_t *dataRemaining;
};
