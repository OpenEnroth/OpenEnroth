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
    bool Initialize();

    const std::string &GetString(LstrId index) const;

    template<class... Args>
    std::string FormatString(LstrId index, Args &&... args) const {
        // TODO(captainurist): what if fmt throws?
        return fmt::sprintf(GetString(index), std::forward<Args>(args)...); // NOLINT: not std::sprintf.
        // TODO(captainurist): there was also a call to sprintfex_internal after a call to vsprintf.
    }

    const std::string &GetDayName(unsigned int index) const {
        return this->day_names[index];
    }

    const std::string &GetMonthName(unsigned int index) const {
        return this->month_names[index];
    }

    const std::string &GetMoonPhaseName(unsigned int index) const {
        return this->moon_phase_names[index];
    }

    const std::string &GetSpellSchoolName(MagicSchool index) const {
        return this->spell_school_names[index];
    }

    const std::string &GetPartyBuffName(PartyBuff index) const {
        return this->party_buff_names[index];
    }

    const std::string &GetCharacterBuffName(CharacterBuff index) const {
        return this->character_buff_names[index];
    }

    const std::string &GetActorBuffName(ActorBuff index) const {
        return this->actor_buff_names[index];
    }

    const std::string &GetClassName(CharacterClass index) const {
        return this->class_names[index];
    }

    const std::string &GetClassDescription(CharacterClass index) const {
        return this->class_desciptions[index];
    }

    const std::string &GetAttirubteName(CharacterAttribute index) const {
        return this->attribute_names[index];
    }

    const std::string &GetAttributeDescription(CharacterAttribute index) const {
        return this->attribute_descriptions[index];
    }

    const std::string &GetSkillName(CharacterSkillType index) const {
        return this->skill_names[index];
    }

    std::string SkillValueShortString(CombinedSkillValue skillValue) const;

    const std::string &MasteryName(CharacterSkillMastery mastery) const {
        switch (mastery) {
        case CHARACTER_SKILL_MASTERY_NOVICE: return GetString(LSTR_NORMAL);
        case CHARACTER_SKILL_MASTERY_EXPERT: return GetString(LSTR_EXPERT);
        case CHARACTER_SKILL_MASTERY_MASTER: return GetString(LSTR_MASTER);
        case CHARACTER_SKILL_MASTERY_GRANDMASTER: return GetString(LSTR_GRAND_1);
        default:
            assert(false);
            return dummy_string;
        }
    }

    const std::string &MasteryNameLong(CharacterSkillMastery mastery) const {
        return mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER ? GetString(LSTR_GRANDMASTER) : MasteryName(mastery);
    }

    const std::string &GetSkillDescription(CharacterSkillType index) const {
        return this->skill_descriptions[index];
    }

    const std::string &GetSkillDescription(CharacterSkillType index, CharacterSkillMastery mastery) const {
        switch(mastery) {
        case CHARACTER_SKILL_MASTERY_NOVICE: return GetSkillDescriptionNormal(index);
        case CHARACTER_SKILL_MASTERY_EXPERT: return GetSkillDescriptionExpert(index);
        case CHARACTER_SKILL_MASTERY_MASTER: return GetSkillDescriptionMaster(index);
        case CHARACTER_SKILL_MASTERY_GRANDMASTER: return GetSkillDescriptionGrand(index);
        default:
            assert(false);
            return dummy_string;
        }
    }

    const std::string &GetSkillDescriptionNormal(CharacterSkillType index) const {
        return this->skill_descriptions_normal[index];
    }

    const std::string &GetSkillDescriptionExpert(CharacterSkillType index) const {
        return this->skill_descriptions_expert[index];
    }

    const std::string &GetSkillDescriptionMaster(CharacterSkillType index) const {
        return this->skill_descriptions_master[index];
    }

    const std::string &GetSkillDescriptionGrand(CharacterSkillType index) const {
        return this->skill_descriptions_grand[index];
    }

    const std::string &GetCharacterConditionName(Condition index) const {
        return this->character_conditions[index];
    }

    const std::string &GetAmPm(bool isPm) const {
        return this->GetString(isPm ? LSTR_PM : LSTR_AM);
    }

    const std::string &GetNpcProfessionName(NpcProfession prof) const {
        return this->npc_profession_names[prof];
    }

    const std::string &GetSpecialAttackName(SpecialAttackType index) const {
        return this->special_attack_names[index];
    }

    const std::string &GetMonsterSpecialAbilityName(MonsterSpecialAbility index) const {
        return this->monster_special_ability_names[index];
    }

    const std::string &getHPDescription() const {
        return this->hp_description;
    }

    const std::string &getSPDescription() const {
        return this->sp_description;
    }

    const std::string &getArmourClassDescription() const {
        return this->armour_class_description;
    }

    const std::string &getCharacterConditionDescription() const {
        return this->character_condition_description;
    }

    const std::string &getFastSpellDescription() const {
        return this->fast_spell_description;
    }

    const std::string &getAgeDescription() const {
        return this->age_description;
    }

    const std::string &getLevelDescription() const {
        return this->level_description;
    }

    const std::string &getExpDescription() const {
        return this->exp_description;
    }

    const std::string &getMeleeAttackDescription() const {
        return this->melee_attack_description;
    }

    const std::string &getMeleeDamageDescription() const {
        return this->melee_damage_description;
    }

    const std::string &getRangedAttackDescription() const {
        return this->ranged_attack_description;
    }

    const std::string &getRangedDamageDescription() const {
        return this->ranged_damage_description;
    }

    const std::string &getFireResistanceDescription() const {
        return this->fire_res_description;
    }

    const std::string &getAirResistanceDescription() const {
        return this->air_res_description;
    }

    const std::string &getWaterResistanceDescription() const {
        return this->water_res_description;
    }

    const std::string &getEarthResistanceDescription() const {
        return this->earth_res_description;
    }

    const std::string &getMindResistanceDescription() const {
        return this->mind_res_description;
    }

    const std::string &getBodyResistanceDescription() const {
        return this->body_res_description;
    }

    const std::string &getSkillPointsDescription() const {
        return this->skill_points_description;
    }

 public:
    Localization() {}

 private:
    void InitializeMm6ItemCategories();

    void InitializeMonthNames();
    void InitializeDayNames();
    void InitializeMoonPhaseNames();

    void InitializeSpellSchoolNames();
    void InitializeSpellNames();

    void InitializeClassNames();
    void InitializeAttributeNames();
    void InitializeSkillNames();
    void InitializeCharacterConditionNames();

    void InitializeNpcProfessionNames();

 private:
    std::string localization_raw;
    IndexedArray<std::string, LSTR_FIRST, LSTR_LAST> localization_strings;
    std::string class_desc_raw;
    std::string attribute_desc_raw;
    std::string skill_desc_raw;

    std::array<std::string, 14> mm6_item_categories;
    std::array<std::string, 12> month_names;
    std::array<std::string, 7> day_names;
    std::array<std::string, 5> moon_phase_names;
    IndexedArray<std::string, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> spell_school_names;
    IndexedArray<std::string, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> party_buff_names;
    IndexedArray<std::string, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> character_buff_names;
    IndexedArray<std::string, ACTOR_BUFF_FIRST, ACTOR_BUFF_LAST> actor_buff_names;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> class_names;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> class_desciptions;
    IndexedArray<std::string, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> attribute_names;
    IndexedArray<std::string, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> attribute_descriptions;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_names;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_normal;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_expert;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_master;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_grand;
    IndexedArray<std::string, CONDITION_FIRST, CONDITION_LAST> character_conditions;
    IndexedArray<std::string, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> npc_profession_names;
    IndexedArray<std::string, SPECIAL_ATTACK_FIRST, SPECIAL_ATTACK_LAST> special_attack_names;
    IndexedArray<std::string, MONSTER_SPECIAL_ABILITY_FIRST, MONSTER_SPECIAL_ABILITY_LAST> monster_special_ability_names;
    IndexedArray<std::string, CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST> skill_value_short_templates;
    std::string hp_description;
    std::string sp_description;
    std::string armour_class_description;
    std::string character_condition_description;
    std::string fast_spell_description;
    std::string age_description;
    std::string level_description;
    std::string exp_description;
    std::string melee_attack_description;
    std::string melee_damage_description;
    std::string ranged_attack_description;
    std::string ranged_damage_description;
    std::string fire_res_description;
    std::string air_res_description;
    std::string water_res_description;
    std::string earth_res_description;
    std::string mind_res_description;
    std::string body_res_description;
    std::string skill_points_description;
    std::string dummy_string;
};

extern Localization *localization;
