#include "MerchantTable.h"

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
    for (auto [cells, i] : TsvReader(merchants).drop(1).skip(&TsvLine::isBlank).zip(allMerchantPhrases())) {
        pMerchantsBuyPhrases[i] = cells[1];
        pMerchantsSellPhrases[i] = cells[2];
        pMerchantsRepairPhrases[i] = cells[3];
        pMerchantsIdentifyPhrases[i] = cells[4];
    }
}
