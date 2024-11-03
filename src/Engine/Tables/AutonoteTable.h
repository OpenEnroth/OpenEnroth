#pragma once

#include <array>

#include "Engine/Data/AutonoteData.h"

class Blob;

/**
 * @offset 0x476750
 */
void initializeAutonotes(const Blob &autonotes);

extern std::array<AutonoteData, 196> pAutonoteTxt;
