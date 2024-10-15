#pragma once

#include <array>

#include "Engine/Data/AwardData.h"

class Blob;

/**
 * @offset 0x4763E0
 */
void initializeAwards(const Blob &awards);

extern std::array<AwardData, 105> pAwards;
