#pragma once

#include <array>
#include <string_view>

#include "Engine/Data/AutonoteData.h"

/**
 * @offset 0x476750
 */
void initializeAutonotes(std::string_view autonotes);

extern std::array<AutonoteData, 300> pAutonoteTxt;
