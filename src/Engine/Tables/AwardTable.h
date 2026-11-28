#pragma once

#include <array>
#include <string_view>

#include "Engine/Data/AwardData.h"
#include "Engine/Data/AwardEnums.h"

#include "Utility/IndexedArray.h"

/**
 * @offset 0x4763E0
 */
void initializeAwards(std::string_view awards);

extern IndexedArray<AwardData, AWARD_FIRST, AWARD_LAST> pAwards;
