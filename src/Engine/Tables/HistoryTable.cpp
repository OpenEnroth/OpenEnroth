#include "HistoryTable.h"

#include <cstring>
#include <string>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

HistoryTable *pHistoryTable;

//----- (00453E6D) --------------------------------------------------------
void HistoryTable::Initialize(const Blob &history) {
    char *test_string;

    std::string txtRaw(history.string_view());
    strtok(txtRaw.data(), "\r");

    historyLines[0].pText = "";
    historyLines[0].pPageTitle = "";
    historyLines[0].uTime = 0;

    for (int i = 0; i < 28; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(test_string, '\t');

        historyLines[i + 1].pText = removeQuotes(tokens[1]);
        historyLines[i + 1].uTime = atoi(tokens[2]);  // strange but in text here string not digit
        historyLines[i + 1].pPageTitle = removeQuotes(tokens[3]);
    }
}
