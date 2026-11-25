#pragma once

#include <array>
#include <string>

#include "Engine/Data/HouseEnums.h"
#include "Engine/Evt/EvtEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/ChestEnums.h"
#include "Engine/Objects/SpriteEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Graphics/FaceEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Data/SoundEnums.h"

class InputStream;

class EvtInstruction {
 public:
    EvtInstruction() = default;
    explicit EvtInstruction(EvtOpcode opcode) : opcode(opcode), data() {}

    /**
     * @param source                    Evt file the instruction comes from, which picks the table its texts come from.
     * @return                          The instruction in readable form, for logging.
     */
    std::string toString(EvtSource source) const;
    static EvtInstruction parse(InputStream &stream, size_t size);

    EvtOpcode opcode = EVENT_Invalid;
    int step = 0;
    int target_step = 0;
    EvtTargetCharacter who = CHOOSE_PLAYER1;
    std::string str;
    union {
        HouseId house_id;
        int chest_id;
        PortraitId portrait_id;
        SpeechId speech_id;
        int text_id;
        Season season;
        int event_id;
        int movie_unknown_field;
        int can_show_npc_dialogue;
        struct {
            std::array<int, 6> random_goto;  // TODO(yoctozepto): only a byte is read in
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
            EvtVariable type;
            int value;
        } variable_descr;
        struct {
            SoundId sound_id;
            int x;
            int y;
        } sound_descr;
        struct {
            DamageType damage_type;
            int damage;
        } damage_descr;
        struct {
            ActorKillCheckPolicy policy;
            int param;
            int num;
            bool invisibleAsDead; // MM8 only, decoded but not consumed: IsNotAlive() already counts Disabled (invisible) actors as dead, and honouring flag 0 breaks out02 ev132.
        } actor_descr;
        struct {
            int id;
            ActorAttribute attr;
            int is_set;
        } actor_flag_descr;
        struct {
            SpellId spell_id;
            Mastery spell_mastery;
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
            DoorAction door_action;
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
            HouseId house_id;
            int exit_pic_id;
            // MM8 stores the house id in two bytes, so its exit picture byte sits one byte later than MM7's. It is 0 on
            // teleports inside a map, 1 on most doors and 3 on a few. Nothing reads it yet, `exit_pic_id` is 0 in MM8.
            int mm8_exit_pic_id;
        } move_map_descr;
        struct {
            int cog;
            FaceAttribute face_bit;
            int is_on;
        } faces_bit_descr;
        struct {
            SpriteId sprite;
            int x;
            int y;
            int z;
            int speed;
            int count;
            bool random_rotate;
        } summon_item_descr;
        struct {
            int npc_id;
            HouseId location_id;
        } npc_move_descr;
        struct {
            int groups_id;
            int group;
        } npc_groups_descr;
        struct {
            ItemTreasureLevel treasure_level;
            RandomItemType treasure_type;
            ItemId item_id;
        } give_item_descr;
        struct {
            int chest_id;
            ChestFlag flag;
            int is_set;
        } chest_flag_descr;
        struct {
            Skill skill_type;
            Mastery skill_mastery; // MASTERY_NONE matches any mastery.
            int skill_level;
        } check_skill_descr;
        struct {
            int text_id;
            int answer_id_a;
            int answer_id_b;
        } input_string_descr; // MM8-only "Question" variant of EVENT_InputString: prompt + two accepted-answer level-string ids; target_step = jump on a match.
        struct {
            ItemId item_id_low;
            ItemId item_id_high;
            int count;
        } check_items_count_descr; // EVENT_CheckItemsCount (+ target_step) / EVENT_RemoveItems.
        struct {
            int min_award;
            int max_award;
        } bounty_award_range_descr; // EVENT_IsTotalBountyHuntingAwardInRange (+ target_step).
        struct {
            int npc_id;
        } npc_in_party_descr; // EVENT_IsNPCInParty (+ target_step).
    } data;
};

