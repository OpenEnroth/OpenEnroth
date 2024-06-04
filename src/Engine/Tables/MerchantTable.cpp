#include "MerchantTable.h"

#include <cstring>
#include <string>

#include "Engine/Objects/NPCEnumFunctions.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsBuyPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsSellPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsRepairPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsIdentifyPhrases;

void initializeMerchants(const Blob &merchants) {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(merchants.string_view());
    strtok(txtRaw.data(), "\r");

    for (MerchantPhrase i : allMerchantPhrases()) {
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
                        pMerchantsBuyPhrases[i] = removeQuotes(test_string);
                        break;
                    case 2:
                        pMerchantsSellPhrases[i] = removeQuotes(test_string);
                        break;
                    case 3:
                        pMerchantsRepairPhrases[i] = removeQuotes(test_string);
                        break;
                    case 4:
                        pMerchantsIdentifyPhrases[i] = removeQuotes(test_string);
                        break;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 5) && !break_loop);
    }
}
