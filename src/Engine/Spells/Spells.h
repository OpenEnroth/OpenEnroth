#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/SpriteEnums.h"
#include "Engine/Time/Duration.h"
#include "Library/Geometry/Vec.h"

#include "Utility/IndexedArray.h"

#include "SpellEnums.h"

class Blob;

struct SpellInfo {
    std::string name;
    std::string pShortName;
    std::string pDescription;
    std::string pBasicSkillDesc;
    std::string pExpertSkillDesc;
    std::string pMasterSkillDesc;
    std::string pGrandmasterSkillDesc;
    DamageType damageType;
    int field_20;
};

struct SpellStats {
    /**
     * @offset 0x45384A
     */
    void Initialize(const Blob &spells);

    IndexedArray<SpellInfo, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> pInfos;
};

class SpellData {
 public:
    SpellData():SpellData(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CHARACTER_SKILL_MASTERY_NONE) {}
    SpellData(int16_t inNormalMana, int16_t inExpertLevelMana,
              int16_t inMasterLevelMana, int16_t inMagisterLevelMana,
              int16_t inNormalLevelRecovery, int16_t inExpertLevelRecovery,
              int16_t inMasterLevelRecovery, int16_t inMagisterLevelRecovery,
              int8_t inBaseDamage, int8_t inBonusSkillDamage, SpellFlags inStats,
              CharacterSkillMastery inSkillMastery);
    IndexedArray<uint16_t, CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST> mana_per_skill;
    IndexedArray<Duration, CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST> recovery_per_skill;
    int8_t baseDamage;
    int8_t bonusSkillDamage;
    SpellFlags flags;
    CharacterSkillMastery skillMastery;
    // char field_12;
    // char field_13;
    // int16_t field_14;
};

struct SpellBookIconPos {
    int32_t Xpos;
    int32_t Ypos;
};

extern SpellStats *pSpellStats;

extern IndexedArray<std::array<struct SpellBookIconPos, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> pIconPos;

extern const IndexedArray<SpriteId, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSpriteMapping;  // 4E3ACC
extern IndexedArray<SpellData, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> pSpellDatas;
extern const IndexedArray<uint16_t, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSoundIds;

/**
 * @offset 0x43AFE3
 */
int CalcSpellDamage(SpellId uSpellID, int spellLevel, CharacterSkillMastery skillMastery, int currentHp);

/**
 * @offset 0x427769
 */
bool IsSpellQuickCastableOnShiftClick(SpellId uSpellID);

/**
 * Function for processing spells cast from game scripts.
 */
void eventCastSpell(SpellId uSpellID, CharacterSkillMastery skillMastery, int skillLevel, Vec3f from, Vec3f to);  // sub_448DF8

void armageddonProgress();
