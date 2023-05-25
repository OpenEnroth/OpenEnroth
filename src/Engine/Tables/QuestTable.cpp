#include "Engine/Tables/QuestTable.h"
#include "Engine/Engine.h"

#include "Utility/String.h"

std::array<const char *, 513> pQuestTable;
std::string pQuestsTXT_Raw;

void initializeQuests() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pQuestsTXT_Raw = engine->_gameResourceManager->getEventsFile("quests.txt").string_view();
    strtok(pQuestsTXT_Raw.data(), "\r");
    memset(pQuestTable.data(), 0, sizeof(pQuestTable));
    for (i = 0; i < 512; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                if (decode_step == 1)
                    pQuestTable[i + 1] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

