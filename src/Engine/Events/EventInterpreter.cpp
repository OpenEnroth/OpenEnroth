#include <string>
#include <utility>
#include <functional>

#include "Engine/Events/EventInterpreter.h"
#include "Engine/Events/EventIR.h"
#include "Engine/Events/Processor.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Engine.h"
#include "Engine/MapInfo.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Utility/Math/TrigLut.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UITransition.h"
#include "GUI/UI/UIStatusBar.h"

/**
 * @offset 0x4465DF
 */
static bool checkSeason(Season season) {
    int monthPlusOne = pParty->uCurrentMonth + 1;
    int daysPlusOne = pParty->uCurrentDayOfMonth + 1;

    switch (season) {
        case SEASON_WINTER:  // winter 12.21 -> 3.20
            return (monthPlusOne == 12 && daysPlusOne >= 21 ||
                    monthPlusOne == 1 || monthPlusOne == 2 ||
                    monthPlusOne == 3 && daysPlusOne <= 20);

        case SEASON_AUTUMN:  // autumn/fall 9.21 -> 12.20
            return (monthPlusOne == 9 && daysPlusOne >= 21 ||
                    monthPlusOne == 10 || monthPlusOne == 11 ||
                    monthPlusOne == 12 && daysPlusOne <= 20);

        case SEASON_SUMMER:  // summer 6.21 -> 9.20
            return (monthPlusOne == 6 && daysPlusOne >= 21 ||
                    monthPlusOne == 7 || monthPlusOne == 8 ||
                    monthPlusOne == 9 && daysPlusOne <= 20);

        case SEASON_SPRING:  // spring 3.21 -> 6.20
            return (monthPlusOne == 3 && daysPlusOne >= 21 ||
                    monthPlusOne == 4 || monthPlusOne == 5 ||
                    monthPlusOne == 6 && daysPlusOne <= 20);

        default:
            assert(false);
            return false;
    }
}

/**
 * @offset 0x448CF4
 */
static void spawnMonsters(int16_t typeindex, int16_t level, int count,
                          Vec3f pos, int group, unsigned int uUniqueName) {
    MapId mapId = pMapStats->GetMapInfo(pCurrentMapName);
    SpawnPoint pSpawnPoint;

    pSpawnPoint.vPosition = pos;
    pSpawnPoint.uGroup = group;
    pSpawnPoint.uRadius = 32;
    pSpawnPoint.uKind = OBJECT_Actor;
    pSpawnPoint.uMonsterIndex = typeindex + 2 * level + level;

    if (mapId != MAP_INVALID) {
        AIDirection direction;
        int oldNumActors = pActors.size();
        SpawnEncounter(&pMapStats->pInfos[mapId], &pSpawnPoint, 0, count, 0);
        Actor::GetDirectionInfo(Pid(OBJECT_Actor, oldNumActors), Pid::character(0), &direction, 1);
        for (int i = oldNumActors; i < pActors.size(); ++i) {
            pActors[i].PrepareSprites(0);
            pActors[i].yawAngle = direction.uYawAngle;
            pActors[i].uniqueNameIndex = uUniqueName;
        }
    }
}

static bool doForChosenPlayer(CharacterChoosePolicy who, RandomEngine *rng, std::function<int(Character&)> func) {
    if (who >= CHOOSE_PLAYER1 && who <= CHOOSE_PLAYER4) {
        return func(pParty->pCharacters[std::to_underlying(who)]);
    } else if (who == CHOOSE_ACTIVE) {
        if (pParty->hasActiveCharacter()) {
            return func(pParty->activeCharacter());
        }
        return false;
    } else if (who == CHOOSE_PARTY) {
        for (Character &player : pParty->pCharacters) {
            if (func(player)) {
                return true;
            }
        }
        return false;
    } else if (who == CHOOSE_RANDOM) {
        return func(pParty->pCharacters[rng->random(4)]);
    }

    assert(false);
    return false;
}

int EventInterpreter::executeOneEvent(int step, bool isNpc) {
    EventIR ir;
    bool stepFound = false;

    for (const EventIR &irTmp : _events) {
        if (irTmp.step == step) {
            ir = irTmp;
            stepFound = true;
            break;
        }
    }

    if (!stepFound) {
        return -1;
    }

    // In NPC mode must process only NPC dialogue related events plus Exit
    if (isNpc) {
        switch (ir.type) {
            case EVENT_Exit:
                return -1;
            case EVENT_OnCanShowDialogItemCmp:
                _readyToExit = true;
                for (Character &player : pParty->pCharacters) {
                    if (player.CompareVariable(ir.data.variable_descr.type, ir.data.variable_descr.value)) {
                        return ir.target_step;
                    }
                }
                break;
            case EVENT_EndCanShowDialogItem:
                return -1;
            case EVENT_SetCanShowDialogItem:
                _readyToExit = true;
                _canShowOption = ir.data.can_show_npc_dialogue;
                break;
            case EVENT_CanShowTopic_IsActorKilled:
                // TODO: enconunter and process
                assert(false);
#if 0
                if (Actor::isActorKilled(ir.data.actor_descr.policy, ir.data.actor_descr.param, ir.data.actor_descr.num)) {
                    return ir.target_step;
                }
#endif
                break;
            default:
                break;
        }
        return step + 1;
    }

    switch (ir.type) {
        case EVENT_Exit:
            return -1;
        case EVENT_SpeakInHouse:
            if (enterHouse(ir.data.house_id)) {
                pAudioPlayer->playHouseSound(SOUND_enter, false);
                HouseId houseId = HOUSE_JAIL;
                if (uCurrentHouse_Animation != 167) { // TODO: magic number
                    houseId = ir.data.house_id;
                }
                createHouseUI(houseId);
            }
            break;
        case EVENT_PlaySound:
            // TODO(captainurist): ir.data.sound_descr.x, ir.data.sound_descr.y used to be passed in.
            pAudioPlayer->playSound(ir.data.sound_descr.sound_id, SOUND_MODE_UI);
            break;
        case EVENT_MouseOver:
            assert(false); // Must be filtered by step in decoder
            break;
        case EVENT_LocationName:
            assert(false); // Must be filtered by step in decoder
            break;
        case EVENT_MoveToMap:
        {
            if (ir.data.move_map_descr.house_id != HOUSE_INVALID || ir.data.move_map_descr.exit_pic_id) {
                pDialogueWindow = new GUIWindow_Transition(ir.data.move_map_descr.house_id, ir.data.move_map_descr.exit_pic_id,
                                                           Vec3f(ir.data.move_map_descr.x, ir.data.move_map_descr.y, ir.data.move_map_descr.z),
                                                           ir.data.move_map_descr.yaw, ir.data.move_map_descr.pitch, ir.data.move_map_descr.zspeed, ir.str);
                savedEventID = _eventId;
                savedEventStep = step + 1;
                return -1;
            }
            engine->_teleportPoint.setTeleportTarget(Vec3f(ir.data.move_map_descr.x, ir.data.move_map_descr.y, ir.data.move_map_descr.z),
                                                     (ir.data.move_map_descr.yaw != -1) ? (ir.data.move_map_descr.yaw & TrigLUT.uDoublePiMask) : -1,
                                                     ir.data.move_map_descr.pitch, ir.data.move_map_descr.zspeed);
            if (ir.str[0] == '0') { // teleport within map
                if (engine->_teleportPoint.isValid()) {
                    engine->_teleportPoint.doTeleport(false);
                    engine->_teleportPoint.invalidate();
                    pAudioPlayer->playUISound(SOUND_teleport);
                }
            } else {
                pGameLoadingUI_ProgressBar->Initialize((GUIProgressBar::Type)((activeLevelDecoration == NULL) + 1));
                Transition_StopSound_Autosave(ir.str, MAP_START_POINT_PARTY);
                _mapExitTriggered = true;
                if (current_screen_type == SCREEN_HOUSE) {
                    if (uGameState == GAME_STATE_CHANGE_LOCATION) {
                        while (houseDialogPressEscape()) {}
                        pMediaPlayer->Unload();
                        window_SpeakInHouse->Release();
                        window_SpeakInHouse = nullptr;
                        engine->_messageQueue->clear();
                        current_screen_type = SCREEN_GAME;
                        if (pDialogueWindow) {
                            pDialogueWindow->Release();
                            pDialogueWindow = 0;
                        }
                    }
                    return -1;
                }
            }
            break;
        }
        case EVENT_OpenChest:
            if (!Chest::open(ir.data.chest_id, _objectPid)) {
                return -1;
            }
            break;
        case EVENT_ShowFace:
            doForChosenPlayer(ir.who, vrng, [&] (Character &player) { player.playEmotion(ir.data.expr_id, 0_ticks); return false; });
            break;
        case EVENT_ReceiveDamage:
            doForChosenPlayer(ir.who, grng, [&] (Character &player) { player.receiveDamage(ir.data.damage_descr.damage, ir.data.damage_descr.damage_type); return false; });
            break;
        case EVENT_SetSnow:
            if (!ir.data.snow_descr.is_nop) {
                pWeather->bRenderSnow = ir.data.snow_descr.is_enable;
            }
            break;
        case EVENT_SetTexture:
            setTexture(ir.data.sprite_texture_descr.cog, ir.str);
            break;
        case EVENT_ShowMovie:
        {
            std::string movieName = trimRemoveQuotes(ir.str);
            if (movieName.length() == 0) {
                break;
            }
            if (pMediaPlayer->IsMoviePlaying()) {
                pMediaPlayer->Unload();
            }

            pMediaPlayer->PlayFullscreenMovie(movieName);

            if (!movieName.compare("arbiter good")) { // change alignment to good
                pParty->alignment = PartyAlignment::PartyAlignment_Good;
                SetUserInterface(pParty->alignment);
            } else if (!movieName.compare("arbiter evil")) { // change alignment to evil
                pParty->alignment = PartyAlignment::PartyAlignment_Evil;
                SetUserInterface(pParty->alignment);
            } else if (!movieName.compare("pcout01")) { // moving to harmondale from emerald isle
                Rest(Duration::fromDays(7));
                pParty->restAndHeal();
                pParty->days_played_without_rest = 0;
            }

            // is this block is needed anymore?
            if (!ir.data.movie_unknown_field || current_screen_type == SCREEN_BOOKS) {
                if (current_screen_type == SCREEN_BOOKS) {
                    pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Fullscreen);
                }

                if (current_screen_type == SCREEN_HOUSE) {
                    pMediaPlayer->OpenHouseMovie(pAnimatedRooms[uCurrentHouse_Animation].video_name, 1);
                }
            }
            break;
        }
        case EVENT_SetSprite:
            setDecorationSprite(ir.data.sprite_texture_descr.cog, ir.data.sprite_texture_descr.hide, ir.str);
            break;
        case EVENT_Compare:
        {
            bool res = doForChosenPlayer(_who, grng, [&] (Character &player) { return player.CompareVariable(ir.data.variable_descr.type, ir.data.variable_descr.value); });
            if (res) {
                return ir.target_step;
            }
            break;
        }
        case EVENT_ChangeDoorState:
            switchDoorAnimation(ir.data.door_descr.door_id, ir.data.door_descr.door_action);
            break;
        case EVENT_Add:
            doForChosenPlayer(_who, grng, [&] (Character &player) { player.AddVariable(ir.data.variable_descr.type, ir.data.variable_descr.value); return false; });
            break;
        case EVENT_Substract:
            if (ir.data.variable_descr.type == VAR_PlayerItemInHands && _who == CHOOSE_PARTY) {
                for (Character &player : pParty->pCharacters) {
                    if (player.hasItem((ItemId)ir.data.variable_descr.value, 1)) {
                        player.SubtractVariable(ir.data.variable_descr.type, ir.data.variable_descr.value);
                        break;  // only take one item
                    }
                }
            } else {
                doForChosenPlayer(_who, grng, [&] (Character &player) { player.SubtractVariable(ir.data.variable_descr.type, ir.data.variable_descr.value); return false; });
            }
            break;
        case EVENT_Set:
            doForChosenPlayer(_who, grng, [&] (Character &player) { player.SetVariable(ir.data.variable_descr.type, ir.data.variable_descr.value); return false; });
            break;
        case EVENT_SummonMonsters:
            spawnMonsters(ir.data.monster_descr.type, ir.data.monster_descr.level, ir.data.monster_descr.count,
                          Vec3f(ir.data.monster_descr.x, ir.data.monster_descr.y, ir.data.monster_descr.z),
                          ir.data.monster_descr.group, ir.data.monster_descr.name_id);
            break;
        case EVENT_CastSpell:
            eventCastSpell(ir.data.spell_descr.spell_id, ir.data.spell_descr.spell_mastery, ir.data.spell_descr.spell_level,
                         Vec3f(ir.data.spell_descr.fromx, ir.data.spell_descr.fromy, ir.data.spell_descr.fromz),
                         Vec3f(ir.data.spell_descr.tox, ir.data.spell_descr.toy, ir.data.spell_descr.toz));
            break;
        case EVENT_SpeakNPC:
            if (_canShowMessages) {
                initializeNPCDialogue(ir.data.npc_descr.npc_id, false);
            } else {
                bDialogueUI_InitializeActor_NPC_ID = ir.data.npc_descr.npc_id;
            }
            break;
        case EVENT_SetFacesBit:
            setFacesBit(ir.data.faces_bit_descr.cog, ir.data.faces_bit_descr.face_bit, ir.data.faces_bit_descr.is_on);
            break;
        case EVENT_ToggleActorFlag:
            Actor::toggleFlag(ir.data.actor_flag_descr.id, ir.data.actor_flag_descr.attr, ir.data.actor_flag_descr.is_set);
            break;
        case EVENT_RandomGoTo:
            return ir.data.random_goto_descr.random_goto[grng->random(ir.data.random_goto_descr.random_goto_len)];
        case EVENT_InputString:
            // Originally starting step was checked to ensure skipping this command when returning from dialogue.
            // Changed to using "step + 1" to go to next event
            //
            // TODO(Nik-RE-dev): this event is not used in MM7. In GrayFace's data it's called "Question" and must have additional arguments
            // that control where events executions must be continued on correct/incorrect input.
            assert(false);
#if 0
            game_ui_status_bar_event_string = (ir.data.text_id < engine->_levelStrings.size()) ? engine->_levelStrings[ir.data.text_id] : "";
            startBranchlessDialogue(_eventId, step + 1, EVENT_InputString);
#endif
            return -1;
        case EVENT_StatusText:
            if (activeLevelDecoration) {
                if (activeLevelDecoration == (LevelDecoration *)1) {
                    current_npc_text = pNPCTopics[ir.data.text_id - 1].pText;
                }
                if (_canShowMessages) {
                    engine->_statusBar->setEvent(pNPCTopics[ir.data.text_id - 1].pText);
                }
            } else {
                if (_canShowMessages) {
                    engine->_statusBar->setEvent((ir.data.text_id < engine->_levelStrings.size()) ? engine->_levelStrings[ir.data.text_id] : "");
                }
            }
            break;
        case EVENT_ShowMessage:
            branchless_dialogue_str.clear();
            if (activeLevelDecoration) {
                current_npc_text = pNPCTopics[ir.data.text_id - 1].pText;
            } else if (ir.data.text_id < engine->_levelStrings.size()) {
                branchless_dialogue_str = engine->_levelStrings[ir.data.text_id];
            }
            break;
        case EVENT_OnTimer:
            // Trigger, must be skipped but can be encountered in vanilla
            return -1;
        case EVENT_ToggleIndoorLight:
            pIndoor->toggleLight(ir.data.light_descr.light_id, ir.data.light_descr.is_enable);
            break;
        case EVENT_PressAnyKey:
            startBranchlessDialogue(_eventId, step + 1, EVENT_PressAnyKey);
            return -1;
        case EVENT_SummonItem:
            SpriteObject::dropItemAt(ir.data.summon_item_descr.sprite, Vec3f(ir.data.summon_item_descr.x, ir.data.summon_item_descr.y, ir.data.summon_item_descr.z),
                                     ir.data.summon_item_descr.speed, ir.data.summon_item_descr.count, ir.data.summon_item_descr.random_rotate);
            break;
        case EVENT_ForPartyMember:
            _who = ir.who;
            break;
        case EVENT_Jmp:
            return ir.target_step;
        case EVENT_OnMapReload:
            // Trigger, must be skipped but can be encountered in vanilla
            return -1;
        case EVENT_OnLongTimer:
            // Trigger, must be skipped but can be encountered in vanilla
            return -1;
        case EVENT_SetNPCTopic:
        {
            NPCData *npc = &pNPCStats->pNPCData[ir.data.npc_topic_descr.npc_id];
            if (ir.data.npc_topic_descr.index == 0) npc->dialogue_1_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.index == 1) npc->dialogue_2_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.index == 2) npc->dialogue_3_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.index == 3) npc->dialogue_4_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.index == 4) npc->dialogue_5_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.index == 5) npc->dialogue_6_evt_id = ir.data.npc_topic_descr.event_id;
            if (ir.data.npc_topic_descr.npc_id == 8) {
                if (ir.data.npc_topic_descr.event_id == 78) {
                    houseDialogPressEscape();
                    window_SpeakInHouse->Release();
                    if (enterHouse(HOUSE_DARK_GUILD_PIT)) {
                        createHouseUI(HOUSE_DARK_GUILD_PIT);
                        current_npc_text = pNPCTopics[90].pText;
                    }
                }
            }
            break;
        }
        case EVENT_MoveNPC:
            pNPCStats->pNPCData[ir.data.npc_move_descr.npc_id].Location2D = ir.data.npc_move_descr.location_id;
            // TODO(Nik-RE-dev): Looks like it's artifact of MM6
#if 0
            if (window_SpeakInHouse) {
                if (window_SpeakInHouse->houseId() == HOUSE_BODY_GUILD_MASTER_ERATHIA) {
                    houseDialogPressEscape();
                    pMediaPlayer->Unload();
                    window_SpeakInHouse->Release();
                    activeLevelDecoration = (LevelDecoration *)1;
                    if (enterHouse(HOUSE_BODY_GUILD_MASTER_ERATHIA)) {
                        pAudioPlayer->playUISound(SOUND_Invalid);
                        window_SpeakInHouse = new GUIWindow_House({0, 0}, render->GetRenderDimensions(), HOUSE_BODY_GUILD_MASTER_ERATHIA, "");
                        window_SpeakInHouse->DeleteButtons();
                    }
                }
            }
#endif
            break;
        case EVENT_GiveItem:
        {
            ItemGen item;
            item.Reset();
            pItemTable->generateItem(ir.data.give_item_descr.treasure_level, ir.data.give_item_descr.treasure_type, &item);
            if (ir.data.give_item_descr.item_id != ITEM_NULL) {
                item.uItemID = ir.data.give_item_descr.item_id;
            }
            pParty->setHoldingItem(&item);
            break;
        }
        case EVENT_ChangeEvent:
            if (ir.data.event_id) {
                engine->_persistentVariables.decorVars[activeLevelDecoration->eventVarId] = ir.data.event_id - 124;
            } else {
                engine->_persistentVariables.decorVars[activeLevelDecoration->eventVarId] = 0;
                activeLevelDecoration->uFlags |= LEVEL_DECORATION_INVISIBLE;
            }
            break;
        case EVENT_CheckSkill:
        {
            assert(_who != CHOOSE_PARTY); // TODO(Nik-RE-dev): original code for this option is dubious
            bool res = doForChosenPlayer(_who, grng, [&] (Character &player) {
                CombinedSkillValue val = player.getSkillValue(ir.data.check_skill_descr.skill_type);
                return val.level() >= ir.data.check_skill_descr.skill_level && val.mastery() == ir.data.check_skill_descr.skill_mastery;
            });
            if (res) {
                return ir.target_step;
            }
            break;
        }
        case EVENT_SetNPCGroupNews:
            pNPCStats->pGroups[ir.data.npc_groups_descr.groups_id] = ir.data.npc_groups_descr.group;
            break;
        case EVENT_SetActorGroup:
            // TODO: enconunter and process
            assert(false);
#if 0
            *(&pActors[0].uGroup + 0x11000000 * _evt->v8 +
              209 * (_evt->v5 +
                     ((_evt->v6 + ((unsigned)_evt->v7 << 8)) << 8))) =
                EVT_DWORD(_evt->v9);
#endif
            break;
        case EVENT_NPCSetItem:
            npcSetItem(ir.data.npc_item_descr.id, ir.data.npc_item_descr.item, ir.data.npc_item_descr.is_give);
            break;
        case EVENT_SetNPCGreeting:
            pNPCStats->pNPCData[ir.data.npc_descr.npc_id].uFlags &= ~NPC_GREETED_FIRST;
            pNPCStats->pNPCData[ir.data.npc_descr.npc_id].uFlags &= ~NPC_GREETED_SECOND;
            pNPCStats->pNPCData[ir.data.npc_descr.npc_id].greet = ir.data.npc_descr.greeting;
            break;
        case EVENT_IsActorKilled:
            if (Actor::isActorKilled(ir.data.actor_descr.policy, ir.data.actor_descr.param, ir.data.actor_descr.num)) {
                return ir.target_step;
            }
            break;
        case EVENT_OnMapLeave:
            // Trigger, must be skipped but can be encountered in vanilla
            return -1;
        case EVENT_ChangeGroup:
            // TODO: enconunter and process
            assert(false);
#if 0
            v38 = EVT_DWORD(_evt->v5);
            v39 = EVT_DWORD(_evt->v9);
            for (unsigned actor_id = 0; actor_id < pActors.size(); actor_id++) {
                if (pActors[actor_id].uGroup == v38)
                    pActors[actor_id].uGroup = v39;
            }
#endif
            break;
        case EVENT_ChangeGroupAlly:
            // TODO: enconunter and process
            assert(false);
#if 0
            v42 = EVT_DWORD(_evt->v5);
            v43 = EVT_DWORD(_evt->v9);
            for (unsigned actor_id = 0; actor_id < pActors.size(); actor_id++) {
                if (pActors[actor_id].uGroup == v42)
                    pActors[actor_id].uAlly = v43;
            }
#endif
            break;
        case EVENT_CheckSeason:
            if (checkSeason(ir.data.season)) {
                return ir.target_step;
            }
            break;
        case EVENT_ToggleActorGroupFlag:
            toggleActorGroupFlag(ir.data.actor_flag_descr.id, ir.data.actor_flag_descr.attr, ir.data.actor_flag_descr.is_set);
            break;
        case EVENT_ToggleChestFlag:
            Chest::toggleFlag(ir.data.chest_flag_descr.chest_id, ir.data.chest_flag_descr.flag, ir.data.chest_flag_descr.is_set);
            break;
        case EVENT_CharacterAnimation:
            doForChosenPlayer(ir.who, vrng, [&] (Character &player) { player.playReaction(ir.data.speech_id); return false; });
            break;
        case EVENT_SetActorItem:
            Actor::giveItem(ir.data.npc_item_descr.id, ir.data.npc_item_descr.item, ir.data.npc_item_descr.is_give);
            break;
        case EVENT_OnDateTimer:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_EnableDateTimer:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_StopAnimation:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_CheckItemsCount:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_RemoveItems:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_SpecialJump:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_IsTotalBountyHuntingAwardInRange:
            // TODO: seems unused
            assert(false);
            break;
        case EVENT_IsNPCInParty:
            // TODO: seems unused
            assert(false);
            break;
        default:
            break;
    }

    return step + 1;
}

bool EventInterpreter::executeRegular(int startStep) {
    assert(startStep >= 0);

    if (!_eventId || !_events.size()) {
        return false;
    }

    int step = startStep;

    _who = !pParty->hasActiveCharacter() ? CHOOSE_RANDOM : CHOOSE_ACTIVE;

    while (step != -1 && dword_5B65C4_cancelEventProcessing == 0) {
        step = executeOneEvent(step, false);
    }

    return _mapExitTriggered;
}

bool EventInterpreter::executeNpcDialogue(int startStep) {
    assert(startStep >= 0);

    if (!_eventId) {
        return false;
    }

    if (!_events.size()) {
        // No event commands found for current eventId
        // In this case dialogue elements can be showed
        return true;
    }

    int step = startStep;

    _who = CHOOSE_PARTY;

    while (step != -1) {
        step = executeOneEvent(step, true);
    }

    // Originally was: "readyToExit ? (canShowOption != 0) : 2"
    return !_readyToExit || _canShowOption;
}

void EventInterpreter::prepare(const EventMap &eventMap, int eventId, Pid objectPid, bool canShowMessages) {
    _eventId = eventId;
    _canShowMessages = canShowMessages;
    _objectPid = objectPid;

    _events.clear();
    if (eventMap.hasEvent(eventId)) {
        _events = eventMap.events(eventId);
    }
}

bool EventInterpreter::isValid() {
    return _events.size() > 0;
}
