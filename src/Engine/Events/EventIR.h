#pragma once

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <string>
#include "Engine/Events/EventEnums.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/ChestEnums.h"
#include "Engine/Objects/SpriteObjectEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Graphics/FaceEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Media/Audio/SoundEnums.h"

enum SPELL_TYPE : uint8_t;
enum SPRITE_OBJECT_TYPE : uint16_t;
enum class ACTOR_KILL_CHECK_POLICY;
enum class ActorAttribute : uint32_t;
enum class CharacterChoosePolicy;
enum class DAMAGE_TYPE : unsigned int;
enum class ITEM_TREASURE_LEVEL : int8_t;
enum class ITEM_TYPE : int32_t;
enum class Season;
enum class VariableType;
enum class ChestFlag : uint16_t;
enum class EventType : uint8_t;
enum class FaceAttribute : uint32_t;
enum class ItemId : int32_t;
enum class ItemTreasureLevel : int8_t;

class EventIR {
 public:
    std::string toString() const;
    static EventIR parse(const void *data, size_t maxSize);

    EventType type;
    int step;
    int target_step;
    CharacterChoosePolicy who;
    std::string str;
    union {
        HOUSE_ID house_id;
        int chest_id;
        CharacterExpressionID expr_id;
        CharacterSpeech speech_id;
        int text_id;
        Season season;
        int event_id;
        int movie_unknown_field;
        int can_show_npc_dialogue;
        struct {
            std::array<int, 6> random_goto;
            int random_goto_len;
        } random_goto_descr;
        struct {
            int npc_id;
            int greeting;
        } npc_descr;
        struct {
            int id;
            ItemId item;
            bool is_give;
        } npc_item_descr;
        struct {
            VariableType type;
            int value;
        } variable_descr;
        struct {
            SoundID sound_id;
            int x;
            int y;
        } sound_descr;
        struct {
            DAMAGE_TYPE damage_type;
            int damage;
        } damage_descr;
        struct {
            ACTOR_KILL_CHECK_POLICY policy;
            int param;
            int num;
        } actor_descr;
        struct {
            int id;
            ActorAttribute attr;
            int is_set;
        } actor_flag_descr;
        struct {
            SPELL_TYPE spell_id;
            CharacterSkillMastery spell_mastery;
            int spell_level;
            int fromx;
            int fromy;
            int fromz;
            int tox;
            int toy;
            int toz;
        } spell_descr;
        struct {
            int type;
            int level;
            int count;
            int x;
            int y;
            int z;
            int group;
            int name_id;
        } monster_descr;
        struct {
            bool is_yearly;
            bool is_monthly;
            bool is_weekly;
            int daily_start_hour;
            int daily_start_minute;
            int daily_start_second;
            int alt_halfmin_interval;
        } timer_descr;
        struct {
            bool is_nop;
            bool is_enable;
        } snow_descr;
        struct {
            int cog;
            int hide;
        } sprite_texture_descr;
        struct {
            int door_id;
            int door_new_state;
        } door_descr;
        struct {
            int light_id;
            int is_enable;
        } light_descr;
        struct {
            int npc_id;
            int index;
            int event_id;
        } npc_topic_descr;
        struct {
            int x;
            int y;
            int z;
            int yaw;
            int pitch;
            int zspeed;
            HOUSE_ID house_id;
            int exit_pic_id;
        } move_map_descr;
        struct {
            int cog;
            FaceAttribute face_bit;
            int is_on;
        } faces_bit_descr;
        struct {
            SPRITE_OBJECT_TYPE sprite;
            int x;
            int y;
            int z;
            int speed;
            int count;
            bool random_rotate;
        } summon_item_descr;
        struct {
            int npc_id;
            int location_id;
        } npc_move_descr;
        struct {
            int groups_id;
            int group;
        } npc_groups_descr;
        struct {
            ItemTreasureLevel treasure_level;
            int treasure_type;
            ItemId item_id;
        } give_item_descr;
        struct {
            int chest_id;
            ChestFlag flag;
            int is_set;
        } chest_flag_descr;
        struct {
            CharacterSkillType skill_type;
            CharacterSkillMastery skill_mastery;
            int skill_level;
        } check_skill_descr;
    } data;
};

