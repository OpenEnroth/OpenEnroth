#include "StorylineTextTable.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

#include "Utility/Memory/Blob.h"
#include "Utility/String.h"

struct StorylineText *pStorylineText;

//----- (00453E6D) --------------------------------------------------------
void StorylineText::Initialize(const Blob &history) {
    char *test_string;

    std::string txtRaw(history.string_view());
    strtok(txtRaw.data(), "\r");

    StoreLine[0].pText = "";
    StoreLine[0].pPageTitle = "";
    StoreLine[0].uTime = 0;
    StoreLine[0].f_9 = 0;
    StoreLine[0].f_A = 0;
    StoreLine[0].f_B = 0;

    for (int i = 0; i < 28; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(test_string, '\t');

        StoreLine[i + 1].pText = removeQuotes(tokens[1]);
        StoreLine[i + 1].uTime = atoi(tokens[2]);  // strange but in text here string not digit
        StoreLine[i + 1].pPageTitle = removeQuotes(tokens[3]);
    }
}
