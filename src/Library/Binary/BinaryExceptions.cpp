#include "BinaryExceptions.h"

#include "Utility/Exception.h"
#include "fmt/core.h"

[[noreturn]] void throwBinarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName) {
    throw Exception("Could not read '{}' from binary stream: expected {} bytes, got only {}", typeName, bytesExpected, bytesRead);
}
