#pragma once

#include "Engine/Objects/CharacterEnums.h"

// TODO(captainurist): This was an attempt at refactoring the various enchantments we had, and it wasn't finished.
//                     Enchantment unification makes sense, so finish this!
struct CEnchantment {
    CEnchantment() {}
    explicit CEnchantment(int bonus, CharacterSkillType skill = CHARACTER_SKILL_INVALID)
        : skillType(skill), statBonus(bonus) {}

    CharacterSkillType skillType = CHARACTER_SKILL_INVALID;
    int statBonus = 0;
};
