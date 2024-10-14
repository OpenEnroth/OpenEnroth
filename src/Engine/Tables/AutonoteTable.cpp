#include "AutonoteTable.h"

#include <cstring>
#include <string>

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

std::array<AutonoteData, 196> pAutonoteTxt;

void initializeAutonotes(const Blob &autonotes) {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(autonotes.string_view());
    strtok(txtRaw.data(), "\r");

    for (int i = 1; i < pAutonoteTxt.size(); ++i) {
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
                switch (decode_step) {
                    case 1:
                        pAutonoteTxt[i].pText = removeQuotes(test_string);
                        break;
                    case 2: {
                        if (ascii::noCaseEquals(test_string, "potion")) {
                            pAutonoteTxt[i].eType = AUTONOTE_POTION_RECIPE;
                            break;
                        }
                        if (ascii::noCaseEquals(test_string, "stat")) {
                            pAutonoteTxt[i].eType = AUTONOTE_STAT_HINT;
                            break;
                        }
                        if (ascii::noCaseEquals(test_string, "seer")) {
                            pAutonoteTxt[i].eType = AUTONOTE_SEER;
                            break;
                        }
                        if (ascii::noCaseEquals(test_string, "obelisk")) {
                            pAutonoteTxt[i].eType = AUTONOTE_OBELISK;
                            break;
                        }
                        if (ascii::noCaseEquals(test_string, "teacher")) {
                            pAutonoteTxt[i].eType = AUTONOTE_TEACHER;
                            break;
                        }
                        pAutonoteTxt[i].eType = AUTONOTE_MISC;
                        break;
                    }
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}

