#include "HistoryTable.h"

#include <array>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

HistoryTable *pHistoryTable;

//----- (00453E6D) --------------------------------------------------------
void HistoryTable::Initialize(const Blob &history) {
    // history.txt table structure: index | text (localized) | notes (localized, not used) | page title (localized).
    historyLines[0].pText = "";
    historyLines[0].pPageTitle = "";

    for (std::string_view line : split(history.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 4> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]);
        historyLines[i].pText = unquote(tokens[1]);
        historyLines[i].pPageTitle = unquote(tokens[3]);
    }
}
