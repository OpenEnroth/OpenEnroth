#include "AutonoteTable.h"

#include <cstring>
#include <map>
#include <string>

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

std::array<AutonoteData, 196> pAutonoteTxt;

void initializeAutonotes(const Blob &autonotes) {
    static const std::map<std::string, AutonoteType, ascii::NoCaseLess> autonoteTypeMap = {
        {"potion", AUTONOTE_POTION_RECIPE},
        {"stat", AUTONOTE_STAT_HINT},
        {"seer", AUTONOTE_SEER},
        {"obelisk", AUTONOTE_OBELISK},
        {"teacher", AUTONOTE_TEACHER},
    };

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(autonotes.str());
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
                    case 2:
                        pAutonoteTxt[i].eType = valueOr(autonoteTypeMap, test_string, AUTONOTE_MISC);
                        break;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}

