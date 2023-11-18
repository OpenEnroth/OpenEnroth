#pragma once

#include "NPCEnums.h"

#include "Utility/Segment.h"

inline Segment<MerchantPhrase> allMerchantPhrases() {
    return {MERCHANT_PHRASE_NOT_ENOUGH_GOLD, MERCAHNT_PHRASE_STOLEN_ITEM};
}
