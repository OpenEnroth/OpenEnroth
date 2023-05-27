#pragma once
#include <string>

#include "Engine/Objects/NPCEnums.h"
#include "Utility/IndexedArray.h"

/**
 * @offset 0x476590
 */
void initializeMerchants();

extern std::string pMerchantsTXT_Raw;
extern IndexedArray<const char *, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsBuyPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsSellPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsRepairPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsIdentifyPhrases;
