#include "BinaryExceptions.h"

#include "Utility/Exception.h"

[[noreturn]] void throwBinarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName) {
    throw Exception("Could not read '{}' from binary stream: expected {} bytes, got only {}", typeName, bytesExpected, bytesRead);
}

[[noreturn]] void throwBinarySerializationLeftoverDataError(size_t bytesRead, size_t bytesLeft, std::string_view typeName) {
    throw Exception("Unexpected data left in binary stream after reading '{}': {} bytes read, {} bytes left", typeName, bytesRead, bytesLeft);
}
