#include <vector>

#include "Engine/Tables/StorylineTextTable.h"

#include "Utility/String.h"
#include "Engine/Engine.h"

struct StorylineText *pStorylineText;

//----- (00453E6D) --------------------------------------------------------
void StorylineText::Initialize() {
    char *test_string;

    std::string txtRaw{ engine->_gameResourceManager->getEventsFile("history.txt").string_view() };
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
