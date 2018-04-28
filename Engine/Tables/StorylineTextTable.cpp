#include "Engine/Tables/StorylineTextTable.h"

#include <vector>

#include "../LOD.h"

char *pHistoryTXT_Raw;
struct StorylineText *pStorylineText;

extern std::vector<char *> Tokenize(char *input, const char separator);

//----- (00453E6D) --------------------------------------------------------
void StorylineText::Initialize() {
    char *test_string;

    free(pHistoryTXT_Raw);
    pHistoryTXT_Raw = (char*)pEvents_LOD->LoadCompressedTexture("history.txt");
    strtok(pHistoryTXT_Raw, "\r");

    StoreLine[0].pText = nullptr;
    StoreLine[0].pPageTitle = nullptr;
    StoreLine[0].uTime = 0;
    StoreLine[0].f_9 = 0;
    StoreLine[0].f_A = 0;
    StoreLine[0].f_B = 0;

    for (int i = 0; i < 28; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        auto tokens = Tokenize(test_string, '\t');

        StoreLine[i + 1].pText = RemoveQuotes(tokens[1]);
        StoreLine[i + 1].uTime =
            atoi(tokens[2]);  // strange but in text here string not digit
        StoreLine[i + 1].pPageTitle = RemoveQuotes(tokens[3]);
    }
}
