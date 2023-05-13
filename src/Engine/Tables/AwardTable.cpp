#include "Engine/Tables/AwardTable.h"
#include "Engine/LOD.h"

#include "Utility/String.h"

std::array<Award, 105> pAwards;
std::string pAwardsTXT_Raw;

void initializeAwards() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pAwardsTXT_Raw = pEvents_LOD->LoadCompressedTexture("awards.txt").string_view();
    strtok(pAwardsTXT_Raw.data(), "\r");

    for (i = 1; i < 105; ++i) {
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
                    pAwards[i].pText = removeQuotes(test_string);
                else if (decode_step == 2)
                    pAwards[i].uPriority = atoi(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}
