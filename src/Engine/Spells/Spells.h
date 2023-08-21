#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/SpriteObjectEnums.h"
#include "Utility/IndexedArray.h"
#include "SpellEnums.h"

class Blob;
enum SPRITE_OBJECT_TYPE : uint16_t;

struct SpellInfo {
    std::string name;
    std::string pShortName;
    std::string pDescription;
    std::string pBasicSkillDesc;
    std::string pExpertSkillDesc;
    std::string pMasterSkillDesc;
    std::string pGrandmasterSkillDesc;
    SPELL_SCHOOL uSchool;
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
              int8_t inBaseDamage, int8_t inBonusSkillDamage, int16_t inStats,
              CharacterSkillMastery inSkillMastery);
    union {
        uint16_t mana_per_skill[4];
        struct {
            uint16_t uNormalLevelMana;
            uint16_t uExpertLevelMana;
            uint16_t uMasterLevelMana;
            uint16_t uMagisterLevelMana;
        };
    };
    union {
        uint16_t recovery_per_skill[4];
        struct {
            uint16_t uNormalLevelRecovery;
            uint16_t uExpertLevelRecovery;
            uint16_t uMasterLevelRecovery;
            uint16_t uMagisterLevelRecovery;
        };
    };
    int8_t baseDamage;
    int8_t bonusSkillDamage;
    int16_t stats;
    CharacterSkillMastery skillMastery;
    // char field_12;
    // char field_13;
    // int16_t field_14;
};

struct SpellBookIconPos {
    int32_t Xpos;
    int32_t Ypos;
};

extern struct SpellStats *pSpellStats;

extern std::array<std::array<struct SpellBookIconPos, 12>, 9> pIconPos;

extern const IndexedArray<SPRITE_OBJECT_TYPE, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSpriteMapping;  // 4E3ACC
extern IndexedArray<SpellData, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> pSpellDatas;
extern const IndexedArray<SPELL_TYPE, ITEM_FIRST_WAND, ITEM_LAST_WAND> wandSpellIds;
extern const IndexedArray<SPELL_TYPE, ITEM_FIRST_SPELL_SCROLL, ITEM_LAST_SPELL_SCROLL> scrollSpellIds;
extern const IndexedArray<SPELL_TYPE, ITEM_FIRST_SPELL_BOOK, ITEM_LAST_SPELL_BOOK> bookSpellIds;
extern const IndexedArray<uint16_t, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSoundIds;

/**
 * @offset 0x43AFE3
 */
int CalcSpellDamage(SPELL_TYPE uSpellID, int spellLevel, CharacterSkillMastery skillMastery, int currentHp);

/**
 * @offset 0x427769
 */
bool IsSpellQuickCastableOnShiftClick(SPELL_TYPE uSpellID);

/**
 * Function for processing spells cast from game scripts.
 */
void eventCastSpell(SPELL_TYPE uSpellID, CharacterSkillMastery skillMastery, int skillLevel, int fromx,
                    int fromy, int fromz, int tox, int toy, int toz);  // sub_448DF8

void armageddonProgress();
