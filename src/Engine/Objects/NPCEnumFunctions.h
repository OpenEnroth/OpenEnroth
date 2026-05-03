#pragma once

#include "NPCEnums.h"

#include "Utility/Segment.h"

inline Segment<MerchantPhrase> allMerchantPhrases() {
    return {MERCHANT_PHRASE_NOT_ENOUGH_GOLD, MERCAHNT_PHRASE_STOLEN_ITEM};
}

inline Segment<NpcProfession> allValidNpcProfessions() {
    return {NPC_PROFESSION_FIRST_VALID, NPC_PROFESSION_LAST_VALID};
}
