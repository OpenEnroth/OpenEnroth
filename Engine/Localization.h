#pragma once
#include "Engine/Strings.h"
#include "Engine/Objects/NPCProf.h"


#define LSTR_AC                          0   // "AC"
#define LSTR_AGE                         5   // "Age"
#define LSTR_ARMOR_CLASS                12   // "Armor Class"
#define LSTR_ATTACK                     18   // "Attack"
#define LSTR_CANCEL                     34   // "Cancel"
#define LSTR_CHOOSE_TARGET              39   // "Choose target"
#define LSTR_CLASS                      41   // "Class"
#define LSTR_COND                       45   // "Cond"
#define LSTR_CONDITION                  47   // "Condition"
#define LSTR_DAY                        56   // "Day"
#define LSTR_DAYS                       57   // "Days"
#define LSTR_PLEASE_REINSTALL           63   // "Might and Magic VII is having trouble loading files. Please re-install to fix this problem. Note: Re-installing will not destroy your save games."
#define LSTR_DMG                        66   // "Dmg"
#define LSTR_EMPTY_SAVESLOT             72   // "Empty"
#define LSTR_ENTER                      73   // "Enter"
#define LSTR_END_CONVERSATION           74   // "End Conversation"
#define LSTR_CALENDAR                   78   // "Calendar"
#define LSTR_DIALOGUE_EXIT              79   // "Exit"
#define LSTR_EXPERIENCE                 83   // "Experience"
#define LSTR_FAME                       84   // "Fame"
#define LSTR_GRAND                      96   // "Grand"
#define LSTR_GOLD                       97   // "Gold"
#define LSTR_RACE_HUMAN                 99   // "Human"
#define LSTR_RACE_ELF                   101  // "Elf"
#define LSTR_RACE_DWARF                 103  // "Dwarf"
#define LSTR_RACE_GOBLIN                106  // "Goblin"
#define LSTR_HP                         107  // "HP"
#define LSTR_HIT_POINTS                 108  // "Hit Points"
#define LSTR_HOUR                       109  // "Hour"
#define LSTR_HOURS                      110  // "Hours"
#define LSTR_PERMANENT                  121  // "Permanent"
#define LSTR_LEVEL                      131  // "Level"
#define LSTR_MAPS                       139  // "Maps"
#define LSTR_RATIONS_FULL               140  // "Your packs are already full!"
#define LSTR_ELIGIBLE_TO_LEVELUP        147  // "You are eligible to train to %u."
#define LSTR_NAME                       149  // "Name"
#define LSTR_NONE                       153  // "None"
#define LSTR_AUTONOTES                  154  // "Autonotes"
#define LSTR_NOT_ENOUGH_GOLD            155  // "You don't have enough gold"
#define LSTR_STAY_IN_THIS_AREA          156  // "Stay in this Area"
#define LSTR_POINTS                     168  // "Points"
#define LSTR_QSPELL                     170  // "QSpell"
#define LSTR_QUICK_SPELL                172  // "Quick Spell"
#define LSTR_CURRENT_QUESTS             174  // "Current Quests"
#define LSTR_REPUTATION                 180  // "Reputation"
#define LSTR_REINSTALL_NECESSARY        184  // "Reinstall Necessary"
#define LSTR_SHOOT                      203  // "Shoot"
#define LSTR_SKILLS                     205  // "Skills"
#define LSTR_SKILL_POINTS               207  // "Skill Points"
#define LSTR_SP                         209  // "SP"
#define LSTR_SPELL_POINTS               212  // "Spell Points"
#define LSTR_GRANDMASTER                225  // "Grandmaster"
#define LSTR_GAMMA_DESCRIPTION          226  // ""Gamma controls the relative ""brightness"" of the game.May vary depending on your monitor.""
#define LSTR_ZOOM_IN                    251  // "Zoom In"
#define LSTR_ZOOM_OUT                   252  // "Zoom Out"
#define LSTR_ARCOMAGE_CARD_DISCARD      266  // "DISCARD A CARD"
#define LSTR_REPUTATION_HATED           379  // "Hated"
#define LSTR_BROTHER                    390  // "brother"
#define LSTR_SISTER                     391  // "sister"
#define LSTR_REPUTATION_UNFRIENDLY      392  // "Unfriendly"
#define LSTR_DAUGHTER                   393  // "daughter"
#define LSTR_UNKNOWN                    394  // "Unknown"
#define LSTR_HIS                        383  // "his"
#define LSTR_HER                        384  // "her"
#define LSTR_SIR_LOWERCASE              385  // "sir"
#define LSTR_SIR                        386  // "Sir"
#define LSTR_LADY_LOWERCASE             387  // "lady"
#define LSTR_LORD                       388  // "Lord"
#define LSTR_LADY                       389  // "Lady"
#define LSTR_MORNING                    395  // "morning"
#define LSTR_DAY                        396  // "day"
#define LSTR_EVENING                    397  // "evening"
#define LSTR_REPUTATION_NEUTRAL         399  // "Neutral"
#define LSTR_REPUTATION_FRIENDLY        402  // "Friendly"
#define LSTR_HIRE                       406  // "Hire"
#define LSTR_HIRE_DETAILS               407  // "Details"
#define LSTR_HIRE_RELEASE               408  // "Release %s"
#define LSTR_SPELL_FAILED               428  // "Spell failed"
#define LSTR_NORMAL                     431  // "Normal"
#define LSTR_MASTER                     432  // "Master"
#define LSTR_EXPERT                     433  // "Expert"
#define LSTR_REPUTATION_RESPECTED       434  // "Respected"
#define LSTR_MINUTES                    436  // "Minutes"
#define LSTR_MINUTE                     437  // "Minute"
#define LSTR_SECONDS                    438  // "Seconds"
#define LSTR_SECOND                     439  // "Second"
#define LSTR_CANT_METEOR_SHOWER_INDOORS 491  // "Can't cast Meteor Shower indoors!"
#define LSTR_CANT_INFERNO_OUTDOORS      492  // "Can't cast Inferno outdoors!"
#define LSTR_CANT_JUMP_AIRBORNE         493  // "Can't cast Jump while airborne!"
#define LSTR_CANT_FLY_INDOORS           494  // "Can't fly indoors"
#define LSTR_CANT_STARBURST_INDOORS     495  // "Can't cast Starburst indoors!"
#define LSTR_NO_VALID_SPELL_TARGET      496  // "No valid target exists!"
#define LSTR_CANT_PRISMATIC_OUTDOORS    497  // "Can't cast Prismatic Light outdoors!"
#define LSTR_CANT_ARMAGEDDON_INDOORS    499  // "Can't cast Armageddon indoors!"
#define LSTR_PC_NAME_RODERIC            506  // "Roderic"
#define LSTR_PC_NAME_ALEXIS             507  // "Alexis"
#define LSTR_PC_NAME_SERENA             508  // "Serena"
#define LSTR_PC_NAME_ZOLTAN             509  // "Zoltan"
#define LSTR_NOTHING_HERE               521  // "Nothing here"
#define LSTR_HIRE_NO_ROOM               533  // "I cannot join you, you're party is full"
#define LSTR_XP_UNTIL_NEXT_LEVEL        538  // "You need %d more experience to train to level %d"
#define LSTR_ARENA_WELCOME              574  // "Welcome to the Arena of Life and Death.  Remember,
                                             // you are only allowed one arena combat per visit.  To
                                             // fight an arena battle, select the option that best
                                             // describes your abilities and return to me- if you
                                             // survive"
#define LSTR_ARENA_REWARD               576  // "Congratulations on your win. Here's your stuff: %u gold."
#define LSTR_ARENA_PREMATURE_EXIT       577  // "Get back in there you wimps"
#define LSTR_ARENA_DIFFICULTY_PAGE      578  // "Page"
#define LSTR_ARENA_DIFFICULTY_SQUIRE    579  // "Squire"
#define LSTR_ARENA_DIFFICULTY_KNIGHT    580  // "Knight"
#define LSTR_ARENA_DIFFICULTY_LORD      581  // "Lord"
#define LSTR_ARENA_AREADY_WON           582  // "You already won this trip to the Arena
#define LSTR_NO_SAVING_IN_ARENA         583  // "No saving in the Arena"
#define LSTR_ITEM_TOO_LAME              585  // "Item is not of high enough quality"
#define LSTR_NOT_ENOUGH_SPELLPOINTS     586  // "Not enough spell points"
#define LSTR_WAND                       595  // "Wand"
#define LSTR_HISTORY                    602  // "History"
#define LSTR_NEW_GAME                   614  // "New Game"
#define LSTR_SAVE_GAME                  615  // "Save Game"
#define LSTR_LOAD_GAME                  616  // "Load Game"
#define LSTR_OPTIONS                    617  // ""Sound, Keyboard, Game Options…""
#define LSTR_QUIT                       618  // "Quit"
#define LSTR_RETURN_TO_GAME             619  // "Return to Game"
#define LSTR_HOSTILE_CREATURES_NEARBY   638  // "There are hostile creatures nearby!"
#define LSTR_SUMMONS_LIMIT_REACHED      648  // "This character can't summon any more monsters!"
#define LSTR_CANT_DO_UNDERWATER         652  // "You can not do that while you are underwater!"
#define LSTR_FOOD                       653  // "Food"
#define LSTR_GAME_SAVED                 656  // "Game Saved!"
#define LSTR_YOURE_DROWNING             660  // "You're drowning!"
#define LSTR_ON_FIRE                    661  // "On fire!"

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

    const char *GetNpcProfessionName(NPCProf prof) const {
        Assert((int)prof >= 0 && (int)prof < 59);
        return this->npc_profession_names[prof];
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

    const char* mm6_item_categories[14]{};
    const char* month_names[12]{};
    const char* day_names[7]{};
    const char* moon_phase_names[5]{};
    const char* spell_school_names[9]{};
    const char* spell_names[44]{};
    const char* class_names[36]{};
    const char* class_desciptions[36]{};
    const char* attribute_names[7]{};
    const char* attribute_descriptions[7]{};
    const char* skill_names[38]{};
    const char* skill_descriptions[38]{};
    const char* skill_descriptions_normal[38]{};
    const char* skill_descriptions_expert[38]{};
    const char* skill_descriptions_master[38]{};
    const char* skill_descriptions_grand[38]{};
    const char* character_conditions[19]{};
    const char* npc_profession_names[59]{};
};

extern Localization *localization;
