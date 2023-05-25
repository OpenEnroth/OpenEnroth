#include "Engine/Tables/TransitionTable.h"

#include "Engine/Engine.h"

std::array<const char *, 465> pTransitionStrings = {{"", nullptr}};
std::string pTransitionsTXT_Raw;

void initializeTransitions() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pTransitionsTXT_Raw = engine->_gameResourceManager->getEventsFile("trans.txt").string_view();
    strtok(pTransitionsTXT_Raw.data(), "\r");

    for (i = 0; i < 464; ++i) {
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
                    pTransitionStrings[i + 1] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

