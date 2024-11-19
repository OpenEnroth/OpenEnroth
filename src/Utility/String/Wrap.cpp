#include "Wrap.h"

#include <string>
#include <vector>

#include "Ascii.h"

std::vector<std::string> wrapText(std::string_view text, size_t size) {
    std::vector<std::string> result;

    size_t breakablePos = 0;
    size_t lineLength = 0;
    size_t lineStart = 0;
    for (size_t i = 0; i <= text.size(); ++i) {
        lineLength++;

        char c = i == text.size() ? '\n' : text[i]; // Simple hack so that we don't have to do weird ifs after the loop.
        if (ascii::isSpace(c))
            breakablePos = i;

        size_t breakPos = static_cast<size_t>(-1);
        size_t startPos = 0;
        if (lineLength > size && breakablePos != 0) {
            /* OK to break! */
            breakPos = breakablePos;
            startPos = breakablePos + 1;
        } else if (lineLength > size && breakablePos == 0) {
            /* Nowhere to break, so break mid-word. */
            breakPos = i;
            startPos = i;
        } else if (c == '\n') {
            /* Forced break. */
            breakPos = i;
            startPos = i + 1;
        }

        if (breakPos != static_cast<size_t>(-1)) {
            result.emplace_back(text.substr(lineStart, breakPos - lineStart));
            breakablePos = 0;
            lineLength = i - startPos + 1;
            lineStart = startPos;
        }
    }

    return result;
}
