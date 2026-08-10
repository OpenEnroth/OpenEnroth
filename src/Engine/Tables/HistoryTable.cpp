#include "HistoryTable.h"

#include <array>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

HistoryTable *pHistoryTable;

//----- (00453E6D) --------------------------------------------------------
void HistoryTable::Initialize(const Blob &history) {
    // history.txt table structure: index | text (localized) | notes (localized, not used) | page title (localized).
    historyLines[0].pText = "";
    historyLines[0].pPageTitle = "";

    for (TsvLine line : TsvReader(history).drop(1).skip(&TsvLine::isEmpty)) {
        int i = line[0].as<int>();
        historyLines[i].pText = line[1];
        historyLines[i].pPageTitle = line[3];
    }
}
