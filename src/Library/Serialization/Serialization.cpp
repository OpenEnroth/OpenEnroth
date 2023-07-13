#include "Serialization.h"

#include <ostream>

void detail::printToStream(const std::string &string, std::ostream *stream) {
    *stream << string;
}
