#include "HistoryTable.h"

#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

HistoryTable *pHistoryTable;

//----- (00453E6D) --------------------------------------------------------
void HistoryTable::Initialize(const Blob &history) {
    // history.txt table structure: index | text (localized) | notes (localized, not used) | page title (localized).
    historyLines[0].pText = "";
    historyLines[0].pPageTitle = "";

    for (TsvLine cells : TsvReader(history).drop(1).skip(&TsvLine::isBlank)) {
        int i = cells[0].as<int>();
        historyLines[i].pText = cells[1];
        historyLines[i].pPageTitle = cells[3];
    }
}
