#include "Engine/Tables/FactionTable.h"

#include "Engine/Engine.h"

#include "../LOD.h"

//----- (004547E4) --------------------------------------------------------
void FactionTable::Initialize() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;
    //  int item_counter;

    pHostileTXT_Raw = pEvents_LOD->LoadCompressedTexture("hostile.txt").string_view();
    strtok(pHostileTXT_Raw.data(), "\r");
    for (i = 0; i < 89; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char*)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                if (decode_step >= 1 && decode_step < 90)
                    relations[decode_step - 1][i] = atoi(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 92) && !break_loop);
    }
    pHostileTXT_Raw.clear();
}
