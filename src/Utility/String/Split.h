#pragma once

#include <string>
#include <algorithm> // For std::find.
#include <vector>

// TODO(captainurist): drop!
std::vector<char*> tokenize(char *input, const char separator);

/**
 * Splits the provided string `s` using separator `sep`, passing `std::string_view` chunks into `consumer`.
 *
 * This function doesn't discard empty chunks, so at least one chunk will always be passed to `consumer` - splitting an
 * empty string produces a single empty chunk.
 *
 * @param s                             String to split.
 * @param sep                           Separator character.
 * @param consumer                      Lambda to pass `std::string_view` chunks into.
 */
template<class Consumer>
void split(std::string_view s, char sep, Consumer &&consumer) {
    const char *pos = s.data();
    const char *end = s.data() + s.size();
    while (pos != end + 1) {
        const char *next = std::find(pos, end, sep);

        consumer(std::string_view(pos, next));
        pos = next + 1;
    }
}

void split(std::string_view s, char sep, std::vector<std::string_view> *result);

inline std::vector<std::string_view> split(std::string_view s, char sep) {
    std::vector<std::string_view> result;
    split(s, sep, &result);
    return result;
}

inline std::vector<std::string_view> split(const char *s, char sep) {
    return split(std::string_view(s), sep);
}

std::vector<std::string_view> split(std::string &&s, char sep) = delete; // Don't dangle!
