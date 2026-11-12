#pragma once

#include <cstdint>
#include <string_view>

#include "Engine/Evt/EvtInstruction.h"

/**
 * A number field of an evt record that is stored as `T`, little endian.
 */
template<class T>
struct EvtWire {};

/**
 * A string field of an evt record, which ends with a zero.
 */
struct EvtWireString {};

/**
 * Stands in for the name and the member of a field that OpenEnroth doesn't keep.
 */
struct EvtPadding {};

/**
 * @param value                         Mastery as an evt record stores it for a spell, 0 to 3 for novice to grandmaster.
 * @return                              The mastery.
 */
inline Mastery evtSpellMastery(uint8_t value) {
    return static_cast<Mastery>(value + 1);
}

/**
 * @param value                         Mastery as an evt record stores it for a skill check, 0 for any mastery and 1 to
 *                                      3 for expert to grandmaster.
 * @return                              The mastery.
 */
inline Mastery evtSkillMastery(uint8_t value) {
    return value == 0 ? MASTERY_NONE : static_cast<Mastery>(value + 1);
}

/**
 * Walks the fields of an instruction in the order that an evt record stores them, after the step and the opcode.
 * The visitor is called once for each field, in one of these forms:
 * - `visitor(EvtWire<T>(), EvtPadding())` for a field that OpenEnroth doesn't keep.
 * - `visitor(EvtWire<T>(), name, member)` for a number field. The member is a number, a bool, an enum, or an array of
 *   numbers that is stored as one `T` for each element.
 * - `visitor(EvtWire<T>(), name, member, convert)` for a number field whose stored value isn't the value of the
 *   member. `convert` takes what is stored and returns what goes into the member.
 * - `visitor(EvtWireString(), name, member)` for a string field.
 *
 * Names are MMExtension's where a script can call the command, so that scripts can pass the fields by name.
 *
 * @param ir                            Instruction to walk, with its opcode set. Can be const.
 * @param visitor                       Visitor to call.
 * @return                              Whether the fields of the opcode are known.
 */
template<class Instruction, class Visitor>
bool visitEvtFields(Instruction &ir, Visitor &&visitor) {
    constexpr EvtWire<uint8_t> u8;
    constexpr EvtWire<uint16_t> u16;
    constexpr EvtWire<uint32_t> u32;
    constexpr EvtWireString str;
    constexpr EvtPadding pad;

    auto &data = ir.data;

    // TODO(yoctozepto): which are present in global events and which in local?
    switch (ir.opcode) {
        case EVENT_Exit:
            visitor(u8, pad); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_SpeakInHouse:
            visitor(u32, "Id", data.house_id);
            return true;
        case EVENT_PlaySound:
            visitor(u32, "Id", data.sound_descr.sound_id);
            visitor(u32, "X", data.sound_descr.x);
            visitor(u32, "Y", data.sound_descr.y);
            return true;
        case EVENT_MouseOver:
            visitor(u8, "Str", data.text_id);
            return true;
        case EVENT_LocationName: // Only in MM6 data.
            visitor(u8, "Str", data.text_id);
            return true;
        case EVENT_MoveToMap:
            visitor(u32, "X", data.move_map_descr.x);
            visitor(u32, "Y", data.move_map_descr.y);
            visitor(u32, "Z", data.move_map_descr.z);
            visitor(u32, "Direction", data.move_map_descr.yaw);
            visitor(u32, "LookAngle", data.move_map_descr.pitch);
            visitor(u32, "SpeedZ", data.move_map_descr.zspeed);
            visitor(u8, "HouseId", data.move_map_descr.house_id); // TODO(captainurist): Is this correct? Houses can have ids > 255.
            visitor(u8, "Icon", data.move_map_descr.exit_pic_id);
            visitor(str, "Name", ir.str);
            return true;
        case EVENT_OpenChest:
            visitor(u8, "Id", data.chest_id);
            return true;
        case EVENT_ShowFace: // TODO(yoctozepto): not present in used MM7 data
            visitor(u8, "Player", ir.who);
            visitor(u8, "Frame", data.portrait_id);
            return true;
        case EVENT_ReceiveDamage:
            visitor(u8, "Player", ir.who);
            visitor(u8, "DamageType", data.damage_descr.damage_type);
            visitor(u32, "Damage", data.damage_descr.damage);
            return true;
        case EVENT_SetSnow: // TODO(yoctozepto): not present in used MM7 data; likely present in MM6
            visitor(u8, "EffectId", data.snow_descr.is_nop);
            visitor(u8, "On", data.snow_descr.is_enable);
            return true;
        case EVENT_SetTexture:
            visitor(u32, "Facet", data.sprite_texture_descr.cog);
            visitor(str, "Name", ir.str);
            return true;
        case EVENT_ShowMovie:
            visitor(u8, pad); // TODO(yoctozepto): always 1 in MM7 data, check MM6&8
            visitor(u8, "ExitCurrentScreen", data.movie_unknown_field); // NOTE(yoctozepto): seems to be a boolean as it takes either 0 or 1 in MM7 data
            visitor(str, "Name", ir.str);
            return true;
        case EVENT_SetSprite:
            visitor(u32, "SpriteId", data.sprite_texture_descr.cog);
            visitor(u8, "Visible", data.sprite_texture_descr.hide);
            visitor(str, "Name", ir.str);
            return true;
        case EVENT_Compare:
        case EVENT_OnCanShowDialogItemCmp:
            visitor(u16, "VarNum", data.variable_descr.type);
            visitor(u32, "Value", data.variable_descr.value);
            visitor(u8, "Jump", ir.target_step);
            return true;
        case EVENT_ChangeDoorState:
            visitor(u8, "Id", data.door_descr.door_id);
            visitor(u8, "State", data.door_descr.door_action);
            return true;
        case EVENT_Add:
        case EVENT_Subtract:
        case EVENT_Set:
            visitor(u16, "VarNum", data.variable_descr.type);
            visitor(u32, "Value", data.variable_descr.value);
            return true;
        case EVENT_SummonMonsters:
            visitor(u8, "TypeIndexInMapStats", data.monster_descr.type);
            visitor(u8, "Level", data.monster_descr.level);
            visitor(u8, "Count", data.monster_descr.count);
            visitor(u32, "X", data.monster_descr.x);
            visitor(u32, "Y", data.monster_descr.y);
            visitor(u32, "Z", data.monster_descr.z);
            visitor(u32, "NPCGroup", data.monster_descr.group);
            visitor(u32, "unk", data.monster_descr.name_id);
            return true;
        case EVENT_CastSpell:
            visitor(u8, "Spell", data.spell_descr.spell_id);
            visitor(u8, "Mastery", data.spell_descr.spell_mastery, &evtSpellMastery);
            visitor(u8, "Skill", data.spell_descr.spell_level);
            visitor(u32, "FromX", data.spell_descr.fromx);
            visitor(u32, "FromY", data.spell_descr.fromy);
            visitor(u32, "FromZ", data.spell_descr.fromz);
            visitor(u32, "ToX", data.spell_descr.tox);
            visitor(u32, "ToY", data.spell_descr.toy);
            visitor(u32, "ToZ", data.spell_descr.toz);
            return true;
        case EVENT_SpeakNPC:
            visitor(u32, "NPC", data.npc_descr.npc_id);
            return true;
        case EVENT_SetFacesBit:
            visitor(u32, "Id", data.faces_bit_descr.cog);
            visitor(u32, "Bit", data.faces_bit_descr.face_bit);
            visitor(u8, "On", data.faces_bit_descr.is_on);
            return true;
        case EVENT_ToggleActorFlag: // TODO(yoctozepto): not present in used MM7 data
            visitor(u32, "Monster", data.actor_flag_descr.id);
            visitor(u32, "Bit", data.actor_flag_descr.attr);
            visitor(u8, "On", data.actor_flag_descr.is_set);
            return true;
        case EVENT_RandomGoTo:
            visitor(u8, "Jumps", data.random_goto_descr.random_goto);
            return true;
        case EVENT_InputString: // TODO(yoctozepto): not present in used MM7 data; likely present in MM8 (e.g., Escaton's riddles)
        case EVENT_StatusText:
        case EVENT_ShowMessage:
            visitor(u32, "Str", data.text_id);
            return true;
        case EVENT_OnTimer:
        case EVENT_OnLongTimer:
            visitor(u8, "EachYear", data.timer_descr.is_yearly);
            visitor(u8, "EachMonth", data.timer_descr.is_monthly);
            visitor(u8, "EachWeek", data.timer_descr.is_weekly);
            visitor(u8, "Hour", data.timer_descr.daily_start_hour);
            visitor(u8, "Minute", data.timer_descr.daily_start_minute);
            visitor(u8, "Second", data.timer_descr.daily_start_second);
            visitor(u16, "HalfMinutes", data.timer_descr.alt_halfmin_interval);
            visitor(u16, pad); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_ToggleIndoorLight:
            visitor(u32, "Id", data.light_descr.light_id);
            visitor(u8, "On", data.light_descr.is_enable);
            return true;
        case EVENT_PressAnyKey: // Only in MM6 data.
            visitor(u8, pad); // Always 0 in MM6 data.
            return true;
        case EVENT_SummonItem: // TODO(yoctozepto): not present in used MM7 data
            visitor(u32, "Type", data.summon_item_descr.sprite);
            visitor(u32, "X", data.summon_item_descr.x);
            visitor(u32, "Y", data.summon_item_descr.y);
            visitor(u32, "Z", data.summon_item_descr.z);
            visitor(u32, "Speed", data.summon_item_descr.speed);
            visitor(u8, "Count", data.summon_item_descr.count);
            visitor(u8, "RandomAngle", data.summon_item_descr.random_rotate);
            return true;
        case EVENT_ForPartyMember:
            visitor(u8, "Player", ir.who);
            return true;
        case EVENT_Jmp:
            visitor(u8, "Jump", ir.target_step);
            return true;
        case EVENT_OnMapReload:
        case EVENT_OnMapLeave:
        case EVENT_EndCanShowDialogItem:
            visitor(u8, pad); // TODO(yoctozepto): always 0 in MM7 data, check MM6&8
            return true;
        case EVENT_SetNPCTopic:
            visitor(u32, "NPC", data.npc_topic_descr.npc_id);
            visitor(u8, "Index", data.npc_topic_descr.index);
            visitor(u32, "Event", data.npc_topic_descr.event_id);
            return true;
        case EVENT_MoveNPC:
            visitor(u32, "NPC", data.npc_move_descr.npc_id);
            visitor(u32, "HouseId", data.npc_move_descr.location_id);
            return true;
        case EVENT_GiveItem:
            visitor(u8, "Strength", data.give_item_descr.treasure_level);
            visitor(u8, "Type", data.give_item_descr.treasure_type);
            visitor(u32, "Id", data.give_item_descr.item_id);
            return true;
        case EVENT_ChangeEvent:
            visitor(u32, "NewEvent", data.event_id);
            return true;
        case EVENT_CheckSkill:
            visitor(u8, "Skill", data.check_skill_descr.skill_type);
            visitor(u8, "Mastery", data.check_skill_descr.skill_mastery, &evtSkillMastery);
            visitor(u32, "Level", data.check_skill_descr.skill_level);
            visitor(u8, "Jump", ir.target_step);
            return true;
        case EVENT_SetCanShowDialogItem:
            visitor(u8, "Visible", data.can_show_npc_dialogue);
            return true;
        case EVENT_SetNPCGroupNews:
            visitor(u32, "NPCGroup", data.npc_groups_descr.groups_id);
            visitor(u32, "NPCNews", data.npc_groups_descr.group);
            return true;
        case EVENT_NPCSetItem:
            visitor(u32, "NPC", data.npc_item_descr.id);
            visitor(u32, "Item", data.npc_item_descr.item);
            visitor(u8, "On", data.npc_item_descr.is_give);
            return true;
        case EVENT_SetActorItem:
            visitor(u32, "Monster", data.npc_item_descr.id);
            visitor(u32, "Item", data.npc_item_descr.item);
            visitor(u8, "Has", data.npc_item_descr.is_give);
            return true;
        case EVENT_SetNPCGreeting:
            visitor(u32, "NPC", data.npc_descr.npc_id);
            visitor(u32, "Greeting", data.npc_descr.greeting);
            return true;
        case EVENT_IsActorKilled:
        case EVENT_CanShowTopic_IsActorKilled: // TODO(yoctozepto): not present in used MM7 data
            visitor(u8, "CheckType", data.actor_descr.policy);
            visitor(u32, "Id", data.actor_descr.param, [&](uint32_t id) -> int {
                return data.actor_descr.policy == KILL_CHECK_MONSTERID ? id + 1 : id; // Evt scripts count monster ids from 0.
            });
            visitor(u8, "Count", data.actor_descr.num);
            visitor(u8, "Jump", ir.target_step);
            return true;
        case EVENT_CheckSeason:
            visitor(u8, "Season", data.season);
            visitor(u8, "Jump", ir.target_step);
            return true;
        case EVENT_ToggleActorGroupFlag:
            visitor(u32, "NPCGroup", data.actor_flag_descr.id);
            visitor(u32, "Bit", data.actor_flag_descr.attr);
            visitor(u8, "On", data.actor_flag_descr.is_set);
            return true;
        case EVENT_ToggleChestFlag:
            visitor(u32, "ChestId", data.chest_flag_descr.chest_id);
            visitor(u32, "Bit", data.chest_flag_descr.flag);
            visitor(u8, "On", data.chest_flag_descr.is_set);
            return true;
        case EVENT_CharacterAnimation:
            visitor(u8, "Player", ir.who);
            visitor(u8, "Animation", data.speech_id);
            return true;
        default:
            return false; // TODO(yoctozepto): the rest aren't present in used MM7 data, and nobody has decoded them.
    }
}
