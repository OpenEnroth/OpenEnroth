#include "MerchantTable.h"

#include <array>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Engine/Objects/NPCEnumFunctions.h"

#include "Utility/Memory/Blob.h"

IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsBuyPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsSellPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsRepairPhrases;
IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsIdentifyPhrases;

void initializeMerchants(const Blob &merchants) {
    // merchant.txt table structure: phrase (localized, not used) | buy (localized) | sell (localized) | repair (localized) | identify (localized).
    for (auto [line, i] : TsvReader(merchants).drop(1).skip(&TsvLine::isBlank).zip(allMerchantPhrases())) {
        pMerchantsBuyPhrases[i] = line[1];
        pMerchantsSellPhrases[i] = line[2];
        pMerchantsRepairPhrases[i] = line[3];
        pMerchantsIdentifyPhrases[i] = line[4];
    }
}
