#include "AutonoteTable.h"

#include <array>
#include <map>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

// Expanded from 196 to 300 to accommodate empty entries in autonote.txt.
std::array<AutonoteData, 300> pAutonoteTxt;

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
        int i = fromString<int>(tokens[0]);
        // TODO(captainurist): We have "0" in autonote texts, and it gets shown. Find out what it was supposed to be.
        pAutonoteTxt[i].pText = tokens[1] == "0" ? "" : removeQuotes(tokens[1]);
        pAutonoteTxt[i].eType = valueOr(autonoteTypeMap, tokens[2], AUTONOTE_MISC);
    }
}
