#pragma once

#include <string>

#include "Engine/Objects/NPCEnums.h"
#include "Utility/IndexedArray.h"

class Blob;

/**
 * @offset 0x476590
 */
void initializeMerchants(const Blob &merchants);

extern IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsBuyPhrases;
extern IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsSellPhrases;
extern IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsRepairPhrases;
extern IndexedArray<std::string, MERCHANT_PHRASE_FIRST, MERCHANT_PHRASE_LAST> pMerchantsIdentifyPhrases;
