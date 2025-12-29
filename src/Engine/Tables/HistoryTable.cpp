#include "HistoryTable.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

HistoryTable *pHistoryTable;

//----- (00453E6D) --------------------------------------------------------
void HistoryTable::Initialize(const Blob &history) {
    std::string txtRaw(history.string_view());
    strtok(txtRaw.data(), "\r");

    historyLines[0].pText = "";
    historyLines[0].pPageTitle = "";
    historyLines[0].uTime = 0;

    for (int i = 0; i < 28; ++i) {
        char *test_string = strtok(nullptr, "\r") + 1;
        std::vector<std::string_view> tokens = split(test_string, '\t');

        historyLines[i + 1].pText = removeQuotes(tokens[1]);
        historyLines[i + 1].uTime = atoi(std::string(tokens[2]).c_str());  // TODO(captainurist): strange but in text here string not digit
        historyLines[i + 1].pPageTitle = removeQuotes(tokens[3]);
    }
}
