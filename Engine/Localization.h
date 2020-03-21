#pragma once
#include "Engine/Strings.h"

#define LSTR_ELIGIBLE_TO_LEVELUP    147 // "You are eligible to train to %u."
#define LSTR_NOT_ENOUGH_GOLD        155 // "You don't have enough gold"
#define LSTR_XP_UNTIL_NEXT_LEVEL    538 // "You need %d more experience to train to level %d"

class Localization {
 public:
    bool Initialize();

    const char *GetString(unsigned int index) const;
    String FormatString(unsigned int index, ...) const;

    const char *GetDayName(unsigned int index) const {
        return this->day_names[index];
    }

    const char *GetMonthName(unsigned int index) const {
        return this->month_names[index];
    }

    const char *GetMoonPhaseName(unsigned int index) const {
        return this->moon_phase_names[index];
    }

    const char *GetSpellSchoolName(unsigned int index) const {
        return this->spell_school_names[index];
    }

    const char *GetSpellName(unsigned int index) const {
        return this->spell_names[index];
    }

    const char *GetClassName(unsigned int index) const {
        return this->class_names[index];
    }

    const char *GetClassDescription(unsigned int index) const {
        return this->class_desciptions[index];
    }

    const char *GetAttirubteName(unsigned int index) const {
        return this->attribute_names[index];
    }

    const char *GetAttributeDescription(unsigned int index) const {
        return this->attribute_descriptions[index];
    }

    const char *GetSkillName(unsigned int index) const {
        return this->skill_names[index];
    }

    const char *GetSkillDescription(unsigned int index) const {
        return this->skill_descriptions[index];
    }

    const char *GetSkillDescriptionNormal(unsigned int index) const {
        return this->skill_descriptions_normal[index];
    }

    const char *GetSkillDescriptionExpert(unsigned int index) const {
        return this->skill_descriptions_expert[index];
    }

    const char *GetSkillDescriptionMaster(unsigned int index) const {
        return this->skill_descriptions_master[index];
    }

    const char *GetSkillDescriptionGrand(unsigned int index) const {
        return this->skill_descriptions_grand[index];
    }

    const char *GetCharacterConditionName(unsigned int index) const {
        return this->character_conditions[index];
    }

    const char *GetAmPm(unsigned int index) const {
        return this->GetString(472 + index);
    }

    const char *GetNpcProfessionName(unsigned int index) const {
        return this->npc_profession_names[index];
    }

 public:
    Localization()
        : localization_raw(nullptr),
          localization_strings(nullptr),
          class_desc_raw(nullptr),
          attribute_desc_raw(nullptr),
          skill_desc_raw(nullptr) {}

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
    char *localization_raw;
    const char **localization_strings;
    char *class_desc_raw;
    char *attribute_desc_raw;
    char *skill_desc_raw;

    const char *mm6_item_categories[14];
    const char *month_names[12];
    const char *day_names[7];
    const char *moon_phase_names[5];
    const char *spell_school_names[9];
    const char *spell_names[44];
    const char *class_names[36];
    const char *class_desciptions[36];
    const char *attribute_names[7];
    const char *attribute_descriptions[7];
    const char *skill_names[38];
    const char *skill_descriptions[38];
    const char *skill_descriptions_normal[38];
    const char *skill_descriptions_expert[38];
    const char *skill_descriptions_master[38];
    const char *skill_descriptions_grand[38];
    const char *character_conditions[19];
    const char *npc_profession_names[59];
};

extern Localization *localization;
