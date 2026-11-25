#include "AutonoteTable.h"

#include <array>
#include <map>
#include <string>

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

// Expanded from 196 to 300 to accommodate empty entries in autonote.txt, then to 301: MM8's autonote.txt has
// a real row at index 300 (blank text, real category) that doesn't fit a 300-slot table.
std::array<AutonoteData, 301> pAutonoteTxt;

void initializeAutonotes(const Blob &autonotes) {
    // autonote.txt table structure: index | text (localized) | type (not localized).
    static const std::map<std::string, AutonoteType, ascii::NoCaseLess> autonoteTypeMap = {
        {"potion", AUTONOTE_POTION_RECIPE},
        {"stat", AUTONOTE_STAT_HINT},
        {"seer", AUTONOTE_SEER},
        {"obelisk", AUTONOTE_OBELISK},
        {"teacher", AUTONOTE_TEACHER},
        {"misc", AUTONOTE_MISC}
    };

    for (std::string_view line : split(autonotes.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 3> tokens = split(line).by('\t'); // Truncated lines with just the index exist, tail defaults to "".
        if (tokens[0].empty())
            continue; // MM8's autonote.txt also has blank-index tab-only trailing lines.
        int i = fromString<int>(tokens[0]);
        // MM8's autonote.txt has real rows past MM7's 300-slot `pAutonoteTxt` ceiling.
        if (i < 0 || static_cast<size_t>(i) >= pAutonoteTxt.size()) {
            MM_WARNING("AutonoteTable: dropping autonote #{}, doesn't fit in a {}-slot table", i, pAutonoteTxt.size());
            continue;
        }
        // TODO(captainurist): We have "0" in autonote texts, and it gets shown. Find out what it was supposed to be.
        pAutonoteTxt[i].pText = tokens[1] == "0" ? "" : unquote(tokens[1]);
        pAutonoteTxt[i].eType = valueOr(autonoteTypeMap, tokens[2], AUTONOTE_MISC);
    }
}
