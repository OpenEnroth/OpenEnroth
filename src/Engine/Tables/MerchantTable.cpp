#include "MerchantTable.h"

#include <array>
#include <string>

#include "Engine/Objects/NPCEnumFunctions.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsBuyPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsSellPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsRepairPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsIdentifyPhrases;

void initializeMerchants(std::string_view merchants) {
    // merchant.txt table structure: phrase (localized, not used) | buy (localized) | sell (localized) | repair (localized) | identify (localized).
    for (auto [line, i] : split(merchants).by("\r\n").drop(1).skip("").zip(allMerchantPhrases())) {
        std::array<std::string_view, 5> tokens = split(line).by('\t');
        pMerchantsBuyPhrases[i] = unquote(tokens[1]);
        pMerchantsSellPhrases[i] = unquote(tokens[2]);
        pMerchantsRepairPhrases[i] = unquote(tokens[3]);
        pMerchantsIdentifyPhrases[i] = unquote(tokens[4]);
    }
}
