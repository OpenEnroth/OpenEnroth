#include "AutonoteTable.h"

#include <array>
#include <map>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"

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

    for (TsvLine line : TsvReader(autonotes).drop(1).skip(&TsvLine::isEmpty)) {
        int i = line[0].as<int>();
        // TODO(captainurist): We have "0" in autonote texts, and it gets shown. Find out what it was supposed to be.
        pAutonoteTxt[i].pText = line[1] == "0" ? std::string_view() : line[1];
        pAutonoteTxt[i].eType = valueOr(autonoteTypeMap, line[2], AUTONOTE_MISC);
    }
}
