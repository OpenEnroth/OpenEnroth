#pragma once

#include <array>
#include <string>
#include <utility>

#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/PartyEnums.h"
#include "Objects/ActorEnums.h"

#include "Utility/IndexedArray.h"
#include "Utility/String/Format.h"

#include "LocalizationEnums.h"
#include "Objects/CombinedSkillValue.h"
#include "Objects/MonsterEnums.h"

class Localization {
 public:
    bool initialize();

    const std::string &str(LstrId index) const;

    template<class... Args>
    std::string format(LstrId index, Args &&... args) const {
        // TODO(captainurist): what if fmt throws?
        return fmt::sprintf(str(index), std::forward<Args>(args)...); // NOLINT: not std::sprintf.
        // TODO(captainurist): there was also a call to sprintfex_internal after a call to vsprintf.
    }

    const std::string &dayName(unsigned int index) const {
        return _dayNames[index];
    }

    const std::string &monthName(unsigned int index) const {
        return _monthNames[index];
    }

    const std::string &moonPhaseName(unsigned int index) const {
        return _moonPhaseNames[index];
    }

    const std::string &spellSchoolName(MagicSchool index) const {
        return _spellSchoolNames[index];
    }

    const std::string &partyBuffName(PartyBuff index) const {
        return _partyBuffNames[index];
    }

    const std::string &characterBuffName(CharacterBuff index) const {
        return _characterBuffNames[index];
    }

    const std::string &actorBuffName(ActorBuff index) const {
        return _actorBuffNames[index];
    }

    const std::string &className(Class index) const {
        return _classNames[index];
    }

    const std::string &classDescription(Class index) const {
        return _classDescriptions[index];
    }

    const std::string &attributeName(Attribute index) const {
        return _attributeNames[index];
    }

    const std::string &attributeDescription(Attribute index) const {
        return _attributeDescriptions[index];
    }

    const std::string &skillName(Skill index) const {
        return _skillNames[index];
    }

    std::string skillValueShortString(CombinedSkillValue skillValue) const;

    const std::string &masteryName(Mastery mastery) const {
        switch (mastery) {
        case MASTERY_NOVICE: return str(LSTR_NORMAL);
        case MASTERY_EXPERT: return str(LSTR_EXPERT);
        case MASTERY_MASTER: return str(LSTR_MASTER);
        case MASTERY_GRANDMASTER: return str(LSTR_GRAND_1);
        default:
            assert(false);
            return _dummyString;
        }
    }

    const std::string &masteryNameLong(Mastery mastery) const {
        return mastery == MASTERY_GRANDMASTER ? str(LSTR_GRANDMASTER) : masteryName(mastery);
    }

    const std::string &skillDescription(Skill index) const {
        return _skillDescriptions[index];
    }

    const std::string &skillDescription(Skill index, Mastery mastery) const {
        switch(mastery) {
        case MASTERY_NOVICE: return skillDescriptionNormal(index);
        case MASTERY_EXPERT: return skillDescriptionExpert(index);
        case MASTERY_MASTER: return skillDescriptionMaster(index);
        case MASTERY_GRANDMASTER: return skillDescriptionGrand(index);
        default:
            assert(false);
            return _dummyString;
        }
    }

    const std::string &skillDescriptionNormal(Skill index) const {
        return _skillDescriptionsNormal[index];
    }

    const std::string &skillDescriptionExpert(Skill index) const {
        return _skillDescriptionsExpert[index];
    }

    const std::string &skillDescriptionMaster(Skill index) const {
        return _skillDescriptionsMaster[index];
    }

    const std::string &skillDescriptionGrand(Skill index) const {
        return _skillDescriptionsGrand[index];
    }

    const std::string &characterConditionName(Condition index) const {
        return _characterConditions[index];
    }

    const std::string &amPm(bool isPm) const {
        return str(isPm ? LSTR_PM : LSTR_AM);
    }

    const std::string &npcProfessionName(NpcProfession prof) const {
        return _npcProfessionNames[prof];
    }

    const std::string &specialAttackName(MonsterSpecialAttack index) const {
        return _specialAttackNames[index];
    }

    const std::string &monsterSpecialAbilityName(MonsterSpecialAbility index) const {
        return _monsterSpecialAbilityNames[index];
    }

    const std::string &hpDescription() const {
        return _hpDescription;
    }

    const std::string &spDescription() const {
        return _spDescription;
    }

    const std::string &armourClassDescription() const {
        return _armourClassDescription;
    }

    const std::string &characterConditionDescription() const {
        return _characterConditionDescription;
    }

    const std::string &fastSpellDescription() const {
        return _fastSpellDescription;
    }

    const std::string &ageDescription() const {
        return _ageDescription;
    }

    const std::string &levelDescription() const {
        return _levelDescription;
    }

    const std::string &expDescription() const {
        return _expDescription;
    }

    const std::string &meleeAttackDescription() const {
        return _meleeAttackDescription;
    }

    const std::string &meleeDamageDescription() const {
        return _meleeDamageDescription;
    }

    const std::string &rangedAttackDescription() const {
        return _rangedAttackDescription;
    }

    const std::string &rangedDamageDescription() const {
        return _rangedDamageDescription;
    }

    const std::string &fireResistanceDescription() const {
        return _fireResDescription;
    }

    const std::string &airResistanceDescription() const {
        return _airResDescription;
    }

    const std::string &waterResistanceDescription() const {
        return _waterResDescription;
    }

    const std::string &earthResistanceDescription() const {
        return _earthResDescription;
    }

    const std::string &mindResistanceDescription() const {
        return _mindResDescription;
    }

    const std::string &bodyResistanceDescription() const {
        return _bodyResDescription;
    }

    const std::string &skillPointsDescription() const {
        return _skillPointsDescription;
    }

 public:
    Localization() = default;

 private:
    void initializeMm6ItemCategories();

    void initializeMonthNames();
    void initializeDayNames();
    void initializeMoonPhaseNames();

    void initializeSpellSchoolNames();
    void initializeSpellNames();

    void initializeClassNames();
    void initializeAttributeNames();
    void initializeSkillNames();
    void initializeCharacterConditionNames();

    void initializeNpcProfessionNames();

 private:
    std::string _localizationRaw;
    IndexedArray<std::string, LSTR_FIRST, LSTR_LAST> _localizationStrings;
    std::string _classDescRaw;
    std::string _attributeDescRaw;
    std::string _skillDescRaw;

    std::array<std::string, 14> _mm6ItemCategories;
    std::array<std::string, 12> _monthNames;
    std::array<std::string, 7> _dayNames;
    std::array<std::string, 5> _moonPhaseNames;
    IndexedArray<std::string, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> _spellSchoolNames;
    IndexedArray<std::string, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> _partyBuffNames;
    IndexedArray<std::string, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> _characterBuffNames;
    IndexedArray<std::string, ACTOR_BUFF_FIRST, ACTOR_BUFF_LAST> _actorBuffNames;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> _classNames;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> _classDescriptions;
    IndexedArray<std::string, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> _attributeNames;
    IndexedArray<std::string, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> _attributeDescriptions;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillNames;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillDescriptions;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillDescriptionsNormal;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillDescriptionsExpert;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillDescriptionsMaster;
    IndexedArray<std::string, SKILL_INVALID, SKILL_LAST_VISIBLE> _skillDescriptionsGrand;
    IndexedArray<std::string, CONDITION_FIRST, CONDITION_LAST> _characterConditions;
    IndexedArray<std::string, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> _npcProfessionNames;
    IndexedArray<std::string, SPECIAL_ATTACK_FIRST, SPECIAL_ATTACK_LAST> _specialAttackNames;
    IndexedArray<std::string, MONSTER_SPECIAL_ABILITY_FIRST, MONSTER_SPECIAL_ABILITY_LAST> _monsterSpecialAbilityNames;
    IndexedArray<std::string, MASTERY_FIRST, MASTERY_LAST> _skillValueShortTemplates;
    std::string _hpDescription;
    std::string _spDescription;
    std::string _armourClassDescription;
    std::string _characterConditionDescription;
    std::string _fastSpellDescription;
    std::string _ageDescription;
    std::string _levelDescription;
    std::string _expDescription;
    std::string _meleeAttackDescription;
    std::string _meleeDamageDescription;
    std::string _rangedAttackDescription;
    std::string _rangedDamageDescription;
    std::string _fireResDescription;
    std::string _airResDescription;
    std::string _waterResDescription;
    std::string _earthResDescription;
    std::string _mindResDescription;
    std::string _bodyResDescription;
    std::string _skillPointsDescription;
    std::string _dummyString;
};

extern Localization *localization;
