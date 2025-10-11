#pragma once

#include <array>

#include "Engine/Data/AwardData.h"
#include "Engine/Data/AwardEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

/**
 * @offset 0x4763E0
 */
void initializeAwards(const Blob &awards);

extern IndexedArray<AwardData, AWARD_FIRST, AWARD_LAST> pAwards;
