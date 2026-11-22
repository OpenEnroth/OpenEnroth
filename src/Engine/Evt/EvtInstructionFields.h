#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

#include "Engine/Evt/EvtInstruction.h"

/**
 * Walks the fields of an instruction in the order that an evt record stores them, after the step and the opcode, and
 * calls `visitor(type, name, member, convert)` for each field:
 * - `type` is a `std::type_identity` of what the record stores, an unsigned integer or an `std::string` that ends
 *   with a zero.
 * - `name` is the name of the field as a `const char *`, MMExtension's where a script can call the command.
 * - `member` is a pointer to the member that holds the field, which is a number, a bool, an enum, a string, or an
 *   array of numbers that the record stores one after another.
 * - `convert` takes what the record stores and returns what goes into the member. It's `std::identity` for most
 *   fields.
 *
 * A field that OpenEnroth doesn't keep is padding. Its name is `nullptr` and its member is a null `void *`.
 *
 * @param ir                            Instruction to walk, with its opcode set. Can be const.
 * @param visitor                       Visitor to call.
 * @return                              Whether the fields of the opcode are known.
 */
template<class Instruction, class Visitor>
bool visitEvtFields(Instruction &ir, Visitor &&visitor) {
    constexpr std::type_identity<uint8_t> u8;
    constexpr std::type_identity<uint16_t> u16;
    constexpr std::type_identity<uint32_t> u32;
    constexpr std::type_identity<std::string> str;

    auto fld = [&](auto type, const char *name, auto *member) {
        visitor(type, name, member, std::identity());
    };
    auto cnv = [&](auto type, const char *name, auto *member, auto &&convert) {
        visitor(type, name, member, convert);
    };
    auto pad = [&](auto type) {
        visitor(type, static_cast<const char *>(nullptr), static_cast<void *>(nullptr), std::identity());
    };

    auto &data = ir.data;

    // TODO(yoctozepto): which are present in global events and which in local?
    switch (ir.opcode) {
        case EVENT_Exit:
            pad(u8); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_SpeakInHouse:
            fld(u32, "Id", &data.house_id);
            return true;
        case EVENT_PlaySound:
            fld(u32, "Id", &data.sound_descr.sound_id);
            fld(u32, "X", &data.sound_descr.x);
            fld(u32, "Y", &data.sound_descr.y);
            return true;
        case EVENT_MouseOver:
            fld(u8, "Str", &data.text_id);
            return true;
        case EVENT_LocationName: // Only in MM6 data.
            fld(u8, "Str", &data.text_id);
            return true;
        case EVENT_MoveToMap:
            fld(u32, "X", &data.move_map_descr.x);
            fld(u32, "Y", &data.move_map_descr.y);
            fld(u32, "Z", &data.move_map_descr.z);
            fld(u32, "Direction", &data.move_map_descr.yaw);
            fld(u32, "LookAngle", &data.move_map_descr.pitch);
            fld(u32, "SpeedZ", &data.move_map_descr.zspeed);
            fld(u8, "HouseId", &data.move_map_descr.house_id); // TODO(captainurist): Is this correct? Houses can have ids > 255.
            fld(u8, "Icon", &data.move_map_descr.exit_pic_id);
            fld(str, "Name", &ir.str);
            return true;
        case EVENT_OpenChest:
            fld(u8, "Id", &data.chest_id);
            return true;
        case EVENT_ShowFace: // TODO(yoctozepto): not present in used MM7 data
            fld(u8, "Player", &ir.who);
            fld(u8, "Frame", &data.portrait_id);
            return true;
        case EVENT_ReceiveDamage:
            fld(u8, "Player", &ir.who);
            fld(u8, "DamageType", &data.damage_descr.damage_type);
            fld(u32, "Damage", &data.damage_descr.damage);
            return true;
        case EVENT_SetSnow: // TODO(yoctozepto): not present in used MM7 data, likely present in MM6
            fld(u8, "EffectId", &data.snow_descr.is_nop);
            fld(u8, "On", &data.snow_descr.is_enable);
            return true;
        case EVENT_SetTexture:
            fld(u32, "Facet", &data.sprite_texture_descr.cog);
            fld(str, "Name", &ir.str);
            return true;
        case EVENT_ShowMovie:
            pad(u8); // TODO(yoctozepto): always 1 in MM7 data, check MM6&8
            fld(u8, "ExitCurrentScreen", &data.movie_unknown_field); // NOTE(yoctozepto): seems to be a boolean as it takes either 0 or 1 in MM7 data
            fld(str, "Name", &ir.str);
            return true;
        case EVENT_SetSprite:
            fld(u32, "SpriteId", &data.sprite_texture_descr.cog);
            fld(u8, "Visible", &data.sprite_texture_descr.hide);
            fld(str, "Name", &ir.str);
            return true;
        case EVENT_Compare:
        case EVENT_OnCanShowDialogItemCmp:
            fld(u16, "VarNum", &data.variable_descr.type);
            fld(u32, "Value", &data.variable_descr.value);
            fld(u8, "Jump", &ir.target_step);
            return true;
        case EVENT_ChangeDoorState:
            fld(u8, "Id", &data.door_descr.door_id);
            fld(u8, "State", &data.door_descr.door_action);
            return true;
        case EVENT_Add:
        case EVENT_Subtract:
        case EVENT_Set:
            fld(u16, "VarNum", &data.variable_descr.type);
            fld(u32, "Value", &data.variable_descr.value);
            return true;
        case EVENT_SummonMonsters:
            fld(u8, "TypeIndexInMapStats", &data.monster_descr.type);
            fld(u8, "Level", &data.monster_descr.level);
            fld(u8, "Count", &data.monster_descr.count);
            fld(u32, "X", &data.monster_descr.x);
            fld(u32, "Y", &data.monster_descr.y);
            fld(u32, "Z", &data.monster_descr.z);
            fld(u32, "NPCGroup", &data.monster_descr.group);
            fld(u32, "unk", &data.monster_descr.name_id);
            return true;
        case EVENT_CastSpell:
            fld(u8, "Spell", &data.spell_descr.spell_id);
            cnv(u8, "Mastery", &data.spell_descr.spell_mastery, [](uint8_t mastery) {
                return static_cast<Mastery>(mastery + 1); // 0 to 3 are novice to grandmaster.
            });
            fld(u8, "Skill", &data.spell_descr.spell_level);
            fld(u32, "FromX", &data.spell_descr.fromx);
            fld(u32, "FromY", &data.spell_descr.fromy);
            fld(u32, "FromZ", &data.spell_descr.fromz);
            fld(u32, "ToX", &data.spell_descr.tox);
            fld(u32, "ToY", &data.spell_descr.toy);
            fld(u32, "ToZ", &data.spell_descr.toz);
            return true;
        case EVENT_SpeakNPC:
            fld(u32, "NPC", &data.npc_descr.npc_id);
            return true;
        case EVENT_SetFacesBit:
            fld(u32, "Id", &data.faces_bit_descr.cog);
            fld(u32, "Bit", &data.faces_bit_descr.face_bit);
            fld(u8, "On", &data.faces_bit_descr.is_on);
            return true;
        case EVENT_ToggleActorFlag: // TODO(yoctozepto): not present in used MM7 data
            fld(u32, "Monster", &data.actor_flag_descr.id);
            fld(u32, "Bit", &data.actor_flag_descr.attr);
            fld(u8, "On", &data.actor_flag_descr.is_set);
            return true;
        case EVENT_RandomGoTo:
            fld(u8, "Jumps", &data.random_goto_descr.random_goto);
            return true;
        case EVENT_InputString: // TODO(yoctozepto): not present in used MM7 data, likely present in MM8 (e.g., Escaton's riddles)
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            fld(u32, "Str", &data.text_id);
            return true;
        case EVENT_OnTimer:
        case EVENT_OnLongTimer:
            fld(u8, "EachYear", &data.timer_descr.is_yearly);
            fld(u8, "EachMonth", &data.timer_descr.is_monthly);
            fld(u8, "EachWeek", &data.timer_descr.is_weekly);
            fld(u8, "Hour", &data.timer_descr.daily_start_hour);
            fld(u8, "Minute", &data.timer_descr.daily_start_minute);
            fld(u8, "Second", &data.timer_descr.daily_start_second);
            fld(u16, "HalfMinutes", &data.timer_descr.alt_halfmin_interval);
            pad(u16); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_ToggleIndoorLight:
            fld(u32, "Id", &data.light_descr.light_id);
            fld(u8, "On", &data.light_descr.is_enable);
            return true;
        case EVENT_PressAnyKey: // Only in MM6 data.
            pad(u8); // Always 0 in MM6 data.
            return true;
        case EVENT_SummonItem: // TODO(yoctozepto): not present in used MM7 data
            fld(u32, "Type", &data.summon_item_descr.sprite);
            fld(u32, "X", &data.summon_item_descr.x);
            fld(u32, "Y", &data.summon_item_descr.y);
            fld(u32, "Z", &data.summon_item_descr.z);
            fld(u32, "Speed", &data.summon_item_descr.speed);
            fld(u8, "Count", &data.summon_item_descr.count);
            fld(u8, "RandomAngle", &data.summon_item_descr.random_rotate);
            return true;
        case EVENT_ForPartyMember:
            fld(u8, "Player", &ir.who);
            return true;
        case EVENT_Jmp:
            fld(u8, "Jump", &ir.target_step);
            return true;
        case EVENT_OnMapReload:
        case EVENT_OnMapLeave:
        case EVENT_EndCanShowDialogItem:
            pad(u8); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_SetNPCTopic:
            fld(u32, "NPC", &data.npc_topic_descr.npc_id);
            fld(u8, "Index", &data.npc_topic_descr.index);
            fld(u32, "Event", &data.npc_topic_descr.event_id);
            return true;
        case EVENT_MoveNPC:
            fld(u32, "NPC", &data.npc_move_descr.npc_id);
            fld(u32, "HouseId", &data.npc_move_descr.location_id);
            return true;
        case EVENT_GiveItem:
            fld(u8, "Strength", &data.give_item_descr.treasure_level);
            fld(u8, "Type", &data.give_item_descr.treasure_type);
            fld(u32, "Id", &data.give_item_descr.item_id);
            return true;
        case EVENT_ChangeEvent:
            fld(u32, "NewEvent", &data.event_id);
            return true;
        case EVENT_CheckSkill:
            fld(u8, "Skill", &data.check_skill_descr.skill_type);
            cnv(u8, "Mastery", &data.check_skill_descr.skill_mastery, [](uint8_t mastery) {
                return mastery == 0 ? MASTERY_NONE : static_cast<Mastery>(mastery + 1); // 0 is any mastery, 1 to 3 are expert to grandmaster.
            });
            fld(u32, "Level", &data.check_skill_descr.skill_level);
            fld(u8, "Jump", &ir.target_step);
            return true;
        case EVENT_SetCanShowDialogItem:
            fld(u8, "Visible", &data.can_show_npc_dialogue);
            return true;
        case EVENT_SetNPCGroupNews:
            fld(u32, "NPCGroup", &data.npc_groups_descr.groups_id);
            fld(u32, "NPCNews", &data.npc_groups_descr.group);
            return true;
        case EVENT_NPCSetItem:
            fld(u32, "NPC", &data.npc_item_descr.id);
            fld(u32, "Item", &data.npc_item_descr.item);
            fld(u8, "On", &data.npc_item_descr.is_give);
            return true;
        case EVENT_SetActorItem:
            fld(u32, "Monster", &data.npc_item_descr.id);
            fld(u32, "Item", &data.npc_item_descr.item);
            fld(u8, "Has", &data.npc_item_descr.is_give);
            return true;
        case EVENT_SetNPCGreeting:
            fld(u32, "NPC", &data.npc_descr.npc_id);
            fld(u32, "Greeting", &data.npc_descr.greeting);
            return true;
        case EVENT_IsActorKilled:
        case EVENT_CanShowTopic_IsActorKilled: // TODO(yoctozepto): not present in used MM7 data
            fld(u8, "CheckType", &data.actor_descr.policy);
            cnv(u32, "Id", &data.actor_descr.param, [&](uint32_t id) -> int {
                return data.actor_descr.policy == KILL_CHECK_MONSTERID ? id + 1 : id; // Evt scripts count monster ids from 0.
            });
            fld(u8, "Count", &data.actor_descr.num);
            fld(u8, "Jump", &ir.target_step);
            return true;
        case EVENT_CheckSeason:
            fld(u8, "Season", &data.season);
            fld(u8, "Jump", &ir.target_step);
            return true;
        case EVENT_ToggleActorGroupFlag:
            fld(u32, "NPCGroup", &data.actor_flag_descr.id);
            fld(u32, "Bit", &data.actor_flag_descr.attr);
            fld(u8, "On", &data.actor_flag_descr.is_set);
            return true;
        case EVENT_ToggleChestFlag:
            fld(u32, "ChestId", &data.chest_flag_descr.chest_id);
            fld(u32, "Bit", &data.chest_flag_descr.flag);
            fld(u8, "On", &data.chest_flag_descr.is_set);
            return true;
        case EVENT_CharacterAnimation:
            fld(u8, "Player", &ir.who);
            fld(u8, "Animation", &data.speech_id);
            return true;
        default:
            return false; // TODO(yoctozepto): the rest aren't present in used MM7 data, and nobody has decoded them.
    }
}
