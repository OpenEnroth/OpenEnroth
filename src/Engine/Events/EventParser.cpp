#include "Engine/Events/EventParser.h"
#include "Engine/Events/EventPrinter.h"

std::map<int, std::vector<EventIR>> localEventsMap;
std::map<int, std::vector<EventIR>> globalEventsMap;

void addEvent(void* pointer, bool isGlobal) {
    _evt_raw *_evt = (_evt_raw*)pointer;
    int id = _evt->v1 + (_evt->v2 << 8);
    EventIR ir;

    ir.type = _evt->_e_type;
    ir.step = _evt->v3;

    switch (ir.type) {
        case EVENT_Exit:
            break;
        case EVENT_SpeakInHouse:
            ir.data.house_id = (enum HOUSE_ID)EVT_DWORD(_evt->v5);
            break;
        case EVENT_PlaySound:
            ir.data.sound_descr.sound_id = (SoundID)EVT_DWORD(_evt->v5);
            ir.data.sound_descr.x = (SoundID)EVT_DWORD(_evt->v9);
            ir.data.sound_descr.y = (SoundID)EVT_DWORD(_evt->v13);
            break;
        case EVENT_MouseOver:
            ir.data.text_id = _evt->v5;
            break;
        case EVENT_LocationName:
            // Nothing?
            break;
        case EVENT_MoveToMap:
            ir.data.move_map_descr.x = EVT_DWORD(_evt->v5);
            ir.data.move_map_descr.y = EVT_DWORD(_evt->v9);
            ir.data.move_map_descr.z = EVT_DWORD(_evt->v13);
            ir.data.move_map_descr.yaw = EVT_DWORD(_evt->v17);
            ir.data.move_map_descr.pitch = EVT_DWORD(_evt->v21);
            ir.data.move_map_descr.zspeed = EVT_DWORD(_evt->v25);
            ir.data.move_map_descr.anim_id = _evt->v29;
            ir.data.move_map_descr.exit_pic_id = _evt->v30;
            ir.str = (char *)&_evt->v31;
            break;
        case EVENT_OpenChest:
            ir.data.chest_id = _evt->v5;
            break;
        case EVENT_ShowFace:
            ir.who = _evt->v5;
            ir.data.expr_id = (CHARACTER_EXPRESSION_ID)_evt->v6;
            break;
        case EVENT_ReceiveDamage:
            ir.data.damage_descr.damage_type = (DAMAGE_TYPE)_evt->v6;
            ir.data.damage_descr.damage = EVT_DWORD(_evt->v7);
            break;
        case EVENT_SetSnow:
            ir.data.snow_descr.is_nop = _evt->v5;
            ir.data.snow_descr.is_enable = _evt->v6;
            break;
        case EVENT_SetTexture:
            ir.data.sprite_texture_descr.cog = EVT_DWORD(_evt->v5);
            ir.str = (char *)&_evt->v9;
            break;
        case EVENT_ShowMovie:
            ir.data.movie_unknown_field = _evt->v6;
            ir.str = (char *)&_evt->v7;
            break;
        case EVENT_SetSprite:
            ir.data.sprite_texture_descr.cog = EVT_DWORD(_evt->v5);
            ir.data.sprite_texture_descr.hide = _evt->v6;
            ir.str = (char *)&_evt->v10;
            break;
        case EVENT_Compare:
            ir.target_step = _evt->v11;
            ir.who = -1;
            ir.data.variable_descr.type = (enum VariableType)EVT_WORD(_evt->v5);
            ir.data.variable_descr.value = EVT_DWORD(_evt->v7);
            break;
        case EVENT_ChangeDoorState:
            ir.data.door_descr.door_id = _evt->v5;
            ir.data.door_descr.door_new_state = _evt->v6;
            break;
        case EVENT_Add:
        case EVENT_Substract:
        case EVENT_Set:
            ir.who = -1;
            ir.data.variable_descr.type = (enum VariableType)EVT_WORD(_evt->v5);
            ir.data.variable_descr.value = EVT_DWORD(_evt->v7);
            break;
        case EVENT_SummonMonsters:
            ir.data.monster_descr.type = _evt->v5;
            ir.data.monster_descr.level = _evt->v6;
            ir.data.monster_descr.count = _evt->v7;
            ir.data.monster_descr.x = EVT_DWORD(_evt->v8);
            ir.data.monster_descr.y = EVT_DWORD(_evt->v12);
            ir.data.monster_descr.z = EVT_DWORD(_evt->v16);
            ir.data.monster_descr.group = EVT_DWORD(_evt->v20);
            ir.data.monster_descr.name_id = EVT_DWORD(_evt->v24);
            break;
        case EVENT_CastSpell:
            ir.data.spell_descr.spell_id = (SPELL_TYPE)_evt->v5;
            ir.data.spell_descr.spell_mastery = (PLAYER_SKILL_MASTERY)(_evt->v6 + 1);
            ir.data.spell_descr.spell_level = _evt->v7;
            ir.data.spell_descr.fromx = EVT_DWORD(_evt->v8);
            ir.data.spell_descr.fromy = EVT_DWORD(_evt->v12);
            ir.data.spell_descr.fromz = EVT_DWORD(_evt->v16);
            ir.data.spell_descr.tox = EVT_DWORD(_evt->v20);
            ir.data.spell_descr.toy = EVT_DWORD(_evt->v24);
            ir.data.spell_descr.toz = EVT_DWORD(_evt->v28);
            break;
        case EVENT_SpeakNPC:
            ir.data.npc_descr.npc_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_SetFacesBit:
            ir.data.faces_bit_descr.cog = EVT_DWORD(_evt->v5);
            ir.data.faces_bit_descr.face_bit = (FaceAttribute)EVT_DWORD(_evt->v9);
            ir.data.faces_bit_descr.is_on = _evt->v13;
            break;
        case EVENT_ToggleActorFlag:
            ir.data.actor_flag_descr.id = EVT_DWORD(_evt->v5);
            ir.data.actor_flag_descr.attr = ActorAttribute(EVT_DWORD(_evt->v9));
            ir.data.actor_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_RandomGoTo:
            ir.data.random_goto_descr.random_goto[0] = _evt->v5;
            ir.data.random_goto_descr.random_goto[1] = _evt->v6;
            ir.data.random_goto_descr.random_goto[2] = _evt->v7;
            ir.data.random_goto_descr.random_goto[3] = _evt->v8;
            ir.data.random_goto_descr.random_goto[4] = _evt->v9;
            ir.data.random_goto_descr.random_goto[5] = _evt->v10;
            ir.data.random_goto_descr.random_goto_len = 1 + !!_evt->v6 + !!_evt->v7 + !!_evt->v8 + !!_evt->v9 + !!_evt->v10;
            break;
        case EVENT_InputString:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_StatusText:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_ShowMessage:
            ir.data.text_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_OnTimer:
            ir.data.timer_descr.years = _evt->v5;
            ir.data.timer_descr.months = _evt->v6;
            ir.data.timer_descr.weeks = _evt->v7;
            ir.data.timer_descr.hours = _evt->v8;
            ir.data.timer_descr.minutes = _evt->v9;
            ir.data.timer_descr.seconds = _evt->v10;
            ir.data.timer_descr.alternative_interval = _evt->v11 + (_evt->v12 << 8);
            break;
        case EVENT_ToggleIndoorLight:
            ir.data.light_descr.light_id = EVT_DWORD(_evt->v5);
            ir.data.light_descr.is_enable = _evt->v9;
            break;
        case EVENT_PressAnyKey:
            // Nothing?
            break;
        case EVENT_SummonItem:
            ir.data.summon_item_descr.sprite = (SPRITE_OBJECT_TYPE)EVT_DWORD(_evt->v5);
            ir.data.summon_item_descr.x = EVT_DWORD(_evt->v9);
            ir.data.summon_item_descr.y = EVT_DWORD(_evt->v13);
            ir.data.summon_item_descr.z = EVT_DWORD(_evt->v17);
            ir.data.summon_item_descr.speed = EVT_DWORD(_evt->v21);
            ir.data.summon_item_descr.count = _evt->v25;
            ir.data.summon_item_descr.random_rotate = (bool)_evt->v26;
            break;
        case EVENT_ForPartyMember:
            ir.who = _evt->v5;
            break;
        case EVENT_Jmp:
            ir.target_step = _evt->v5;
            break;
        case EVENT_OnMapReload:
            // Nothing?
            break;
        case EVENT_OnLongTimer:
            ir.data.timer_descr.years = _evt->v5;
            ir.data.timer_descr.months = _evt->v6;
            ir.data.timer_descr.weeks = _evt->v7;
            ir.data.timer_descr.hours = _evt->v8;
            ir.data.timer_descr.minutes = _evt->v9;
            ir.data.timer_descr.seconds = _evt->v10;
            ir.data.timer_descr.alternative_interval = _evt->v11 + (_evt->v12 << 8);
            break;
        case EVENT_SetNPCTopic:
            ir.data.npc_topic_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_topic_descr.index = _evt->v9;
            ir.data.npc_topic_descr.event_id = EVT_DWORD(_evt->v10);
            break;
        case EVENT_MoveNPC:
            ir.data.npc_move_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_move_descr.location_id = EVT_DWORD(_evt->v9);
            break;
        case EVENT_GiveItem:
            ir.data.give_item_descr.treasure_level = (ITEM_TREASURE_LEVEL)_evt->v5;
            ir.data.give_item_descr.treasure_type = _evt->v6;
            ir.data.give_item_descr.item_id = (ITEM_TYPE)EVT_DWORD(_evt->v7);
            break;
        case EVENT_ChangeEvent:
            ir.data.event_id = EVT_DWORD(_evt->v5);
            break;
        case EVENT_CheckSkill:
            ir.data.check_skill_descr.skill_type = (PLAYER_SKILL_TYPE)_evt->v5;
            ir.data.check_skill_descr.skill_mastery = (PLAYER_SKILL_MASTERY)_evt->v6;
            ir.data.check_skill_descr.skill_level = (PLAYER_SKILL_LEVEL)EVT_DWORD(_evt->v7);
            ir.target_step = _evt->v11;
            break;
        case EVENT_OnCanShowDialogItemCmp:
            // TODO
            break;
        case EVENT_EndCanShowDialogItem:
            // TODO
            break;
        case EVENT_SetCanShowDialogItem:
            // TODO
            break;
        case EVENT_SetNPCGroupNews:
            ir.data.npc_groups_descr.groups_id = EVT_DWORD(_evt->v5);
            ir.data.npc_groups_descr.group = EVT_DWORD(_evt->v9);
            break;
        case EVENT_SetActorGroup:
            // TODO
            break;
        case EVENT_NPCSetItem:
            ir.data.npc_item_descr.id = EVT_DWORD(_evt->v5);
            ir.data.npc_item_descr.item = (ITEM_TYPE)EVT_DWORD(_evt->v9);
            ir.data.npc_item_descr.is_give = _evt->v13;
            break;
        case EVENT_SetNPCGreeting:
            ir.data.npc_descr.npc_id = EVT_DWORD(_evt->v5);
            ir.data.npc_descr.greeting = EVT_DWORD(_evt->v9);
            break;
        case EVENT_IsActorAlive:
            ir.data.actor_descr.type = _evt->v5;
            ir.data.actor_descr.param = EVT_DWORD(_evt->v6);
            ir.data.actor_descr.num = _evt->v10;
            break;
        case EVENT_IsActorAssasinated:
            // TODO
            break;
        case EVENT_OnMapLeave:
            // Nothing?
            break;
        case EVENT_ChangeGroup:
            // TODO
            break;
        case EVENT_ChangeGroupAlly:
            // TODO
            break;
        case EVENT_CheckSeason:
            ir.data.season = _evt->v5;
            ir.target_step = _evt->v6;
            break;
        case EVENT_ToggleActorGroupFlag:
            ir.data.actor_flag_descr.id = EVT_DWORD(_evt->v5);
            ir.data.actor_flag_descr.attr = ActorAttribute(EVT_DWORD(_evt->v9));
            ir.data.actor_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_ToggleChestFlag:
            ir.data.chest_flag_descr.chest_id = EVT_DWORD(_evt->v5);
            ir.data.chest_flag_descr.flag = (CHEST_FLAG)EVT_DWORD(_evt->v9);
            ir.data.chest_flag_descr.is_set = _evt->v13;
            break;
        case EVENT_CharacterAnimation:
            ir.who = _evt->v5;
            ir.data.speech_id = (PlayerSpeech)_evt->v6;
            break;
        case EVENT_SetActorItem:
            ir.data.npc_item_descr.id = EVT_DWORD(_evt->v5);
            ir.data.npc_item_descr.item = (ITEM_TYPE)EVT_DWORD(_evt->v9);
            ir.data.npc_item_descr.is_give = _evt->v13;
            break;
        case EVENT_OnDateTimer:
            // TODO
            break;
        case EVENT_EnableDateTimer:
            // TODO
            break;
        case EVENT_StopAnimation:
            // TODO
            break;
        case EVENT_CheckItemsCount:
            // TODO
            break;
        case EVENT_RemoveItems:
            // TODO
            break;
        case EVENT_SpecialJump:
            // TODO
            break;
        case EVENT_IsTotalBountyHuntingAwardInRange:
            // TODO
            break;
        case EVENT_IsNPCInParty:
            // TODO
            break;
        default:
            break;
    }

    std::map<int, std::vector<EventIR>> &eventsMap = isGlobal ? globalEventsMap : localEventsMap;

    if (eventsMap.contains(id)) {
        eventsMap[id].push_back(ir);
    } else {
        std::vector<EventIR> newEvtList;
        newEvtList.push_back(ir);
        eventsMap[id] = newEvtList;
    }
}

void clearEvents(bool isGlobal) {
    if (isGlobal) {
        globalEventsMap.clear();
    } else {
        localEventsMap.clear();
    }
}
