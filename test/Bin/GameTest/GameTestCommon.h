#pragma once

#include <vector>

#include "Engine/Objects/CharacterEnums.h"

struct CharacterPreset {
    Class classType;
    Race race;
};

/**
 * Sets up a battle test on Emerald Island:
 * - Moves the party in front of the bridge;
 * - Applies wizard's eye;
 * - Configures one character per entry in `presets` (class & race), killing off the rest;
 * - Chonks each configured character (level / endurance / bodybuilding) so they don't die in combat;
 * - Zeroes out luck on each configured character so that luck rolls don't randomly bump damage dealt to them.
 */
void prepareForBattleTest(const std::vector<CharacterPreset> &presets = {{CLASS_KNIGHT, RACE_GOBLIN}});
