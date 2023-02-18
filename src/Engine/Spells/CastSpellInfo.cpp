#include "Engine/Spells/CastSpellInfo.h"

#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/stru123.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"
#include "Library/Random/Random.h"

using EngineIoc = Engine_::IocContainer;

static SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

const size_t CastSpellInfoCount = 10;
std::array<CastSpellInfo, CastSpellInfoCount> pCastSpellInfo;

/**
 * Common initialization of SpriteObject for spell casting
 *
 * Correct field uType of spritePtr must be set before calling this function
 * because initialization depends on it.
 */
static void InitSpellSprite(SpriteObject *spritePtr,
                            PLAYER_SKILL_LEVEL spellLevel,
                            PLAYER_SKILL_MASTERY spellMastery,
                            CastSpellInfo *pCastSpell) {
    assert(spritePtr && spritePtr->uType != SPRITE_NULL);
    assert(pCastSpell->uSpellID != 0);

    spritePtr->containing_item.Reset();
    spritePtr->spell_level = spellLevel;
    spritePtr->spell_id = pCastSpell->uSpellID;
    spritePtr->spell_skill = spellMastery;
    spritePtr->uAttributes = 0;
    spritePtr->uSpriteFrameID = 0;
    spritePtr->uObjectDescID = pObjectList->ObjectIDByItemID(spritePtr->uType);
    spritePtr->spell_caster_pid = PID(OBJECT_Player, pCastSpell->uPlayerID);
    spritePtr->uSoundID = pCastSpell->sound_id;
}

/**
 * Notify that spell casting failed.
 */
static void SpellFailed(CastSpellInfo *pCastSpell,
                        int error_str_id) {
    GameUI_SetStatusBar(error_str_id);
    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
    pCastSpell->uSpellID = 0;
}

/**
 * Cast spell processing.
 *
 * @offset 0x00427E01
 */
void CastSpellInfoHelpers::CastSpell() {
    CastSpellInfo *pCastSpell;
    int monster_id;
    int spell_overlay_id;
    int dist_X;
    int dist_Y;
    int dist_Z;
    int spell_spray_angle_start;
    int spell_spray_angle_end;
    int spell_speed;
    int target_undead;
    int spellduration;

    PLAYER_SKILL_TYPE which_skill;
    AIDirection target_direction;
    PARTY_BUFF_INDEX buff_resist;

    bool spell_sound_flag = false;

    Player *pPlayer;
    ObjectType obj_type;
    SpriteObject pSpellSprite;

    static const int ONE_THIRD_PI = TrigLUT.uIntegerPi / 3;

    for (int n = 0; n < CastSpellInfoCount; ++n) {  // cycle through spell queue
        pCastSpell = &pCastSpellInfo[n];
        if (pCastSpell->uSpellID == 0) {
            continue;  // spell item blank skip to next
        }

        if (pParty->Invisible()) {
            // casting a spell breaks invisibility
            pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (pCastSpell->uFlags & ON_CAST_CastingInProgress) {
            if (!pParty->pPlayers[pCastSpell->uPlayerID].CanAct()) {
                // this cancels the spell cast if the player can no longer act
                Cancel_Spell_Cast_In_Progress();
            }
            continue;
        }

        pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID];

        int spell_targeted_at = pCastSpell->spell_target_pid;

        if (spell_targeted_at == 0) {  // no target ?? test
            if (pCastSpell->uSpellID == SPELL_LIGHT_DESTROY_UNDEAD ||
                pCastSpell->uSpellID == SPELL_SPIRIT_TURN_UNDEAD ||
                pCastSpell->uSpellID == SPELL_DARK_CONTROL_UNDEAD) {
                target_undead = 1;
            } else {
                target_undead = 0;
            }

            // find the closest target
            spell_targeted_at = stru_50C198.FindClosestActor(engine->config->gameplay.RangedAttackDepth.Get(), 1, target_undead);

            int spell_pointed_target = mouse->uPointingObjectID;

            if (mouse->uPointingObjectID &&
                PID_TYPE(spell_pointed_target) == OBJECT_Actor &&
                pActors[PID_ID(spell_pointed_target)].CanAct()) {  // check can act
                spell_targeted_at = mouse->uPointingObjectID;
            }
        }

        pSpellSprite.uType = spell_sprite_mapping[pCastSpell->uSpellID].uSpriteType;

        if (pSpellSprite.uType != SPRITE_NULL) {
            if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                Actor::GetDirectionInfo(
                    PID(OBJECT_Player, pCastSpell->uPlayerID + 1),
                    spell_targeted_at, &target_direction,
                    0);  // target direciton
            } else {
                target_direction.uYawAngle = pParty->sRotationZ;  // spray infront of party
                target_direction.uPitchAngle = pParty->sRotationY;
            }
        }

        PLAYER_SKILL_LEVEL spell_level;
        PLAYER_SKILL_MASTERY spell_mastery;
        if (pCastSpell->forced_spell_skill_level) {
            // for spell scrolls - decode spell power and mastery
            spell_level = GetSkillLevel(pCastSpell->forced_spell_skill_level);
            spell_mastery = GetSkillMastery(pCastSpell->forced_spell_skill_level);
        } else {
            if (pCastSpell->uSpellID < SPELL_AIR_WIZARD_EYE) {
                which_skill = PLAYER_SKILL_FIRE;
            } else if (pCastSpell->uSpellID < SPELL_WATER_AWAKEN) {
                which_skill = PLAYER_SKILL_AIR;
            } else if (pCastSpell->uSpellID < SPELL_EARTH_STUN) {
                which_skill = PLAYER_SKILL_WATER;
            } else if (pCastSpell->uSpellID < SPELL_SPIRIT_DETECT_LIFE) {
                which_skill = PLAYER_SKILL_EARTH;
            } else if (pCastSpell->uSpellID < SPELL_MIND_REMOVE_FEAR) {
                which_skill = PLAYER_SKILL_SPIRIT;
            } else if (pCastSpell->uSpellID < SPELL_BODY_CURE_WEAKNESS) {
                which_skill = PLAYER_SKILL_MIND;
            } else if (pCastSpell->uSpellID < SPELL_LIGHT_LIGHT_BOLT) {
                which_skill = PLAYER_SKILL_BODY;
            } else if (pCastSpell->uSpellID < SPELL_DARK_REANIMATE) {
                which_skill = PLAYER_SKILL_LIGHT;
            } else if (pCastSpell->uSpellID < SPELL_BOW_ARROW) {
                which_skill = PLAYER_SKILL_DARK;
            } else if (pCastSpell->uSpellID == SPELL_BOW_ARROW) {
                which_skill = PLAYER_SKILL_BOW;
            } else if (pCastSpell->uSpellID == SPELL_101 ||
                pCastSpell->uSpellID == SPELL_LASER_PROJECTILE) {
                which_skill = PLAYER_SKILL_BLASTER;
            } else {
                assert(false && "Unknown spell");
            }

            spell_level = pPlayer->GetActualSkillLevel(which_skill);
            spell_mastery = pPlayer->GetActualSkillMastery(which_skill);

            if (engine->config->debug.AllMagic.Get()) {
                spell_level = 10;
                spell_mastery = PLAYER_SKILL_MASTERY_GRANDMASTER;
            }
        }

        if (pCastSpell->uSpellID < SPELL_BOW_ARROW) {
            if (pCastSpell->forced_spell_skill_level || engine->config->debug.AllMagic.Get()) {
                uRequiredMana = 0;
            } else {
                uRequiredMana = pSpellDatas[pCastSpell->uSpellID].mana_per_skill[std::to_underlying(spell_mastery) - 1];
            }

            sRecoveryTime = pSpellDatas[pCastSpell->uSpellID].recovery_per_skill[std::to_underlying(spell_mastery) - 1];
        }

        if (!pCastSpell->forced_spell_skill_level) {
            if (which_skill == PLAYER_SKILL_DARK && pParty->uCurrentHour == 0 && pParty->uCurrentMinute == 0 ||
                which_skill == PLAYER_SKILL_LIGHT && pParty->uCurrentHour == 12 && pParty->uCurrentMinute == 0) {  // free spells at midnight or midday
                uRequiredMana = 0;
            }
        }

        if (pCastSpell->uSpellID < SPELL_BOW_ARROW && pPlayer->sMana < uRequiredMana) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_SPELLPOINTS);
            pCastSpell->uSpellID = 0;
            continue;
        }

        if (pPlayer->IsCursed() && pCastSpell->uSpellID < SPELL_BOW_ARROW && grng->Random(100) < 50) {  // player is cursed and have a chance to fail spell casting
            if (!pParty->bTurnBasedModeOn) {
                pPlayer->SetRecoveryTime((int64_t)(debug_non_combat_recovery_mul * flt_debugrecmod3 * 100.0));
            } else {
                pParty->pTurnBasedPlayerRecoveryTimes[pCastSpellInfo[n].uPlayerID] = 100;
                pPlayer->SetRecoveryTime(sRecoveryTime);
                pTurnEngine->ApplyPlayerAction();
            }

            SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
            pPlayer->SpendMana(uRequiredMana);
            return;
        }

        assert(pCastSpell->uSpellID != SPELL_101 && "Unknown spell effect #101 (prolly flaming bow arrow");

        // First process special "pseudo" spells like bow or blaster shots
        // and spells that open additional menus like town portal or lloyd beacon
        if (pCastSpell->uSpellID == SPELL_BOW_ARROW) {
            int amount = 1;
            if (spell_mastery >= PLAYER_SKILL_MASTERY_MASTER) {
                amount = 2;
            }

            sRecoveryTime = pPlayer->GetAttackRecoveryTime(true);
            InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            if (pPlayer->WearsItem(ITEM_ARTIFACT_ULLYSES, ITEM_SLOT_BOW)) {
                pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(0xBD6u);
            }
            pSpellSprite.vPosition.x = pParty->vPosition.x;
            pSpellSprite.vPosition.y = pParty->vPosition.y;
            pSpellSprite.vPosition.z = pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
            pSpellSprite.spell_target_pid = spell_targeted_at;
            pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
            pSpellSprite.uFacing = target_direction.uYawAngle;
            memcpy(&pSpellSprite.containing_item, &pPlayer->pInventoryItemList[pPlayer->pEquipment.uBow - 1], sizeof(pSpellSprite.containing_item));
            pSpellSprite.uAttributes |= SPRITE_MISSILE;
            if (pParty->bTurnBasedModeOn) {
                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
            }
            for (int i = 0; i < amount; ++i) {
                if (i) {
                    pSpellSprite.vPosition.z += 32;
                }
                pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                            pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
            }
        } else if (pCastSpell->uSpellID == SPELL_LASER_PROJECTILE) {
            sRecoveryTime = pPlayer->GetAttackRecoveryTime(false);
            InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            // TODO(pskelton): was pParty->uPartyHeight / 2
            pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
            pSpellSprite.spell_target_pid = spell_targeted_at;
            pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
            pSpellSprite.uFacing = target_direction.uYawAngle;
            memcpy(&pSpellSprite.containing_item, &pPlayer->pInventoryItemList[pPlayer->pEquipment.uMainHand - 1], sizeof(pSpellSprite.containing_item));
            // &pParty->pPlayers[pCastSpell->uPlayerID].spellbook.pDarkSpellbook.bIsSpellAvailable[36
            // *
            // pParty->pPlayers[pCastSpell->uPlayerID].pEquipment.uMainHand +
            // 5], );
            pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
            pSpellSprite.uAttributes |= SPRITE_MISSILE;
            if (pParty->bTurnBasedModeOn) {
                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
            }
            if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                        pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                ++pTurnEngine->pending_actions;
            }
        } else if (pCastSpell->uSpellID == SPELL_WATER_TOWN_PORTAL) {
            if (pPlayer->CanCastSpell(uRequiredMana)) {
                int amount = 10 * spell_level;
                if (pParty->uFlags & (PARTY_FLAGS_1_ALERT_RED |
                            PARTY_FLAGS_1_ALERT_YELLOW) &&
                        spell_mastery != PLAYER_SKILL_MASTERY_GRANDMASTER ||
                        grng->Random(100) >= amount && spell_mastery != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                    SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                    continue;
                }
                TownPortalCasterId = pCastSpell->uPlayerID;
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OnCastTownPortal, 0, 0);
                spell_sound_flag = true;
            } else {
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = 0;
                continue;
            }
        } else if (pCastSpell->uSpellID == SPELL_WATER_LLOYDS_BEACON) {
            if (pCurrentMapName == "d05.blv") {  // Arena
                SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                continue;
            }
            if (pPlayer->CanCastSpell(uRequiredMana)) {
                pEventTimer->Pause();
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OnCastLloydsBeacon, 0, 0);
                _506348_current_lloyd_playerid = pCastSpell->uPlayerID;
                ::uRequiredMana = uRequiredMana;
                ::sRecoveryTime = sRecoveryTime;
                LloydsBeaconSpellLevel = (int)(604800 * spell_level);
                LloydsBeaconSpellId = pCastSpell->uSpellID;
                pCastSpell->uFlags |= ON_CAST_NoRecoverySpell;
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OnCastLloydsBeacon, 0, 0);
            } else {
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = 0;
                continue;
            }
        } else {
            int amount;
            Vec3i spell_velocity = Vec3i(0, 0, 0);

            if (!pPlayer->CanCastSpell(uRequiredMana)) {
                // Not enough mana for current spell, check the next one
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = 0;
                continue;
            }

            switch (pCastSpell->uSpellID) {
                case SPELL_FIRE_TORCH_LIGHT:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 4;
                            break;
                        default:
                            assert(false);
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].Apply(
                            GameTime(pParty->GetPlayingTime() + (GameTime::FromSeconds(3600 * spell_level))),
                            spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_FIRE_FIRE_SPIKE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 9;
                            break;
                        default:
                            assert(false);
                    }
                    int spikes_active = 0;
                    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
                        SpriteObject *object = &pSpriteObjects[i];
                        if (object->uType &&
                                object->spell_id == SPELL_FIRE_FIRE_SPIKE &&
                                object->spell_caster_pid == PID(OBJECT_Player, pCastSpell->uPlayerID)) {
                            ++spikes_active;
                        }
                    }
                    if (spikes_active > amount) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = (short)target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, pParty->sRotationY + 10, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_AIR_IMPLOSION:
                {
                    monster_id = PID_ID(spell_targeted_at);
                    if (!spell_targeted_at) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = 0;
                        pSpellSprite.uFacing = 0;
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z + pActors[monster_id].uActorHeight / 2;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    }
                    break;
                }

                case SPELL_EARTH_MASS_DISTORTION:
                {
                    monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Apply(GameTime(pMiscTimer->uTotalGameTimeElapsed + 128), PLAYER_SKILL_MASTERY_NONE, 0, 0, 0);
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = 0;
                        pSpellSprite.uFacing = 0;
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z;
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    }
                    break;
                }

                case SPELL_LIGHT_DESTROY_UNDEAD:
                {
                    if (!spell_targeted_at ||
                            PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        break;
                    }
                    // v730 = spell_targeted_at >> 3;
                    // HIDWORD(spellduration) =
                    // (int)&pActors[PID_ID(spell_targeted_at)];
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition.x = pActors[PID_ID(spell_targeted_at)].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[PID_ID(spell_targeted_at)].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[PID_ID(spell_targeted_at)].vPosition.z;
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    int obj_id = pSpellSprite.Create(0, 0, 0, 0);
                    if (!MonsterStats::BelongsToSupertype(pActors[PID_ID(spell_targeted_at)].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    Actor::DamageMonsterFromParty(PID(OBJECT_Item, obj_id), PID_ID(spell_targeted_at), &spell_velocity);
                    break;
                }

                case SPELL_FIRE_FIRE_BOLT:
                case SPELL_FIRE_FIREBALL:
                case SPELL_FIRE_INCINERATE:
                case SPELL_AIR_LIGHNING_BOLT:
                case SPELL_WATER_ICE_BOLT:
                case SPELL_WATER_ICE_BLAST:
                case SPELL_EARTH_STUN:
                case SPELL_EARTH_DEADLY_SWARM:
                case SPELL_MIND_MIND_BLAST:
                case SPELL_MIND_PSYCHIC_SHOCK:
                case SPELL_BODY_HARM:
                case SPELL_LIGHT_LIGHT_BOLT:
                case SPELL_DARK_DRAGON_BREATH:
                {
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    } else {
                        pSpellSprite.uSectorID = 0;
                    }
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pCastSpell->uSpellID == SPELL_AIR_LIGHNING_BOLT) {
                        pSpellSprite.uAttributes |= SPRITE_SKIP_A_FRAME;
                    }
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_WATER_ACID_BURST:
                case SPELL_EARTH_BLADES:
                case SPELL_BODY_FLYING_FIST:
                case SPELL_DARK_TOXIC_CLOUD:
                {
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    // TODO(pskelton): was pParty->uPartyHeight / 2
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_LIGHT_SUNRAY:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor ||
                            uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
                            (pParty->uCurrentHour < 5 || pParty->uCurrentHour >= 21)) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_LIGHT_PARALYZE:
                {
                    monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor &&
                            pActors[monster_id].DoesDmgTypeDoDamage(DMGT_LIGHT)) {
                        Actor::AI_Stand(PID_ID(spell_targeted_at), 4, 0x80, 0);
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_PARALYZED].Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(3 * 60 * spell_level)), spell_mastery, 0, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        pActors[monster_id].vVelocity.x = 0;
                        pActors[monster_id].vVelocity.y = 0;
                        spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(&pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_EARTH_SLOW:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 180 * spell_level;
                            amount = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 300 * spell_level;
                            amount = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 300 * spell_level;
                            amount = 4;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 300 * spell_level;
                            amount = 8;
                            break;
                        default:
                            assert(false);
                    }
                    // v721 = 836 * PID_ID(spell_targeted_at);
                    monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor && pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_SLOWED].Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)), spell_mastery, amount, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(&pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_MIND_CHARM:
                {
                    monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                        uint power = 300 * spell_level;
                        if (spell_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
                            power = 600 * spell_level;
                        } else if (spell_mastery == PLAYER_SKILL_MASTERY_MASTER) {
                            power = 29030400;
                        }

                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(power << 7)), spell_mastery, 0, 0, 0);
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z + pActors[monster_id].uActorHeight;
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_DARK_SHRINKING_RAY:
                {
                    // spell_id different?
                    InitSpellSprite(&pSpellSprite, spell_level * 300, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.spell_id = SPELL_FIRE_PROTECTION_FROM_FIRE;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_DARK_VAMPIRIC_WEAPON:
                case SPELL_FIRE_FIRE_AURA:
                {
                    ItemGen *item = &pParty->pPlayers[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    item->UpdateTempBonus(pParty->GetPlayingTime());
                    if (item->uItemID == ITEM_BLASTER || item->uItemID == ITEM_BLASTER_RIFLE ||
                            item->IsBroken() || pItemTable->IsMaterialNonCommon(item) || item->special_enchantment != ITEM_ENCHANTMENT_NULL || item->uEnchantmentType != 0 ||
                            !IsWeapon(item->GetItemEquipType())) {
                        _50C9D0_AfterEnchClickEventId = 113;
                        _50C9D4_AfterEnchClickEventSecondParam = 0;
                        _50C9D8_AfterEnchClickEventTimeout = 128; // was 1, increased to make message readable
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }

                    switch (pCastSpell->uSpellID) {
                        case SPELL_FIRE_FIRE_AURA:
                            switch (spell_mastery) {
                                case PLAYER_SKILL_MASTERY_NOVICE:
                                    item->special_enchantment = ITEM_ENCHANTMENT_OF_FIRE;
                                    break;
                                case PLAYER_SKILL_MASTERY_EXPERT:
                                    item->special_enchantment = ITEM_ENCHANTMENT_OF_FLAME;
                                    break;
                                case PLAYER_SKILL_MASTERY_MASTER:
                                case PLAYER_SKILL_MASTERY_GRANDMASTER:
                                    item->special_enchantment = ITEM_ENCHANTMENT_OF_INFERNOS;
                                    break;
                                default:
                                    __debugbreak();
                            }

                            item->uAttributes |= ITEM_AURA_EFFECT_RED;
                            break;
                        case SPELL_DARK_VAMPIRIC_WEAPON:
                            item->special_enchantment = ITEM_ENCHANTMENT_VAMPIRIC;
                            item->uAttributes |= ITEM_AURA_EFFECT_PURPLE;
                            break;
                        default:
                            __debugbreak();
                    }

                    if (spell_mastery < PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spellduration = 3600 * spell_level;
                        item->uExpireTime = GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration));
                        item->uAttributes |= ITEM_TEMP_BONUS;
                    }

                    _50C9A8_item_enchantment_timer = 256;
                    break;
                }

                case SPELL_BODY_REGENERATION:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 10;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_REGENERATION]
                        .Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(3600 * spell_level)), spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_FIRE_PROTECTION_FROM_FIRE:
                case SPELL_AIR_PROTECTION_FROM_AIR:
                case SPELL_WATER_PROTECTION_FROM_WATER:
                case SPELL_EARTH_PROTECTION_FROM_EARTH:
                case SPELL_MIND_PROTECTION_FROM_MIND:
                case SPELL_BODY_PROTECTION_FROM_BODY:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = std::to_underlying(spell_mastery) * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    switch (pCastSpell->uSpellID) {
                        case SPELL_FIRE_PROTECTION_FROM_FIRE:
                            buff_resist = PARTY_BUFF_RESIST_FIRE;
                            break;
                        case SPELL_AIR_PROTECTION_FROM_AIR:
                            buff_resist = PARTY_BUFF_RESIST_AIR;
                            break;
                        case SPELL_WATER_PROTECTION_FROM_WATER:
                            buff_resist = PARTY_BUFF_RESIST_WATER;
                            break;
                        case SPELL_EARTH_PROTECTION_FROM_EARTH:
                            buff_resist = PARTY_BUFF_RESIST_EARTH;
                            break;
                        case SPELL_MIND_PROTECTION_FROM_MIND:
                            buff_resist = PARTY_BUFF_RESIST_MIND;
                            break;
                        case SPELL_BODY_PROTECTION_FROM_BODY:
                            buff_resist = PARTY_BUFF_RESIST_BODY;
                            break;
                        default:
                            assert(false);
                            continue;
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);

                    pParty->pPartyBuffs[buff_resist].Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(3600 * spell_level)), spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_FIRE_HASTE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 60 * (spell_level + 60);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 60 * (spell_level + 60);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 180 * (spell_level + 20);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 240 * (spell_level + 15);
                            break;
                        default:
                            assert(false);
                    }

                    bool has_weak = false;
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        if (pParty->pPlayers[pl_id].conditions.Has(Condition_Weak)) {
                            has_weak = true;
                        }
                    }
                    if (has_weak) {
                        // If any chanracter is in weak state, spell has no effect but mana is
                        // spent
                        // TODO: Vanilla bug here? See: https://www.celestialheavens.com/forum/10/7196
                        pPlayer->SpendMana(uRequiredMana);
                        continue;
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)), spell_mastery, 0, 0, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    break;
                }

                case SPELL_SPIRIT_BLESS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 900 * (spell_level + 4);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 3600 * (spell_level + 1);
                            break;
                        default:
                            assert(false);
                    }
                    amount = spell_level + 5;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        spell_overlay_id = pOtherOverlayList->_4418B1(10000, pCastSpell->uPlayerID_2 + 310, 0, 65536);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_BLESS]
                            .Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)), spell_mastery, amount, spell_overlay_id, 0);
                    } else {
                        for (uint pl_id = 0; pl_id < 4; pl_id++) {
                            spell_fx_renderer->SetPlayerBuffAnim(
                                    pCastSpell->uSpellID, pl_id);
                            spell_overlay_id = pOtherOverlayList->_4418B1(
                                    10000, pl_id + 310, 0, 65536);
                            pParty->pPlayers[pl_id].pPlayerBuffs[PLAYER_BUFF_BLESS]
                                .Apply(GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                                        spell_mastery, amount, spell_overlay_id, 0);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_SPIRIT_LASH:
                {
                    if (spell_targeted_at &&
                            PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        int monster_id = PID_ID(spell_targeted_at);
                        dist_X = abs(pActors[monster_id].vPosition.x - pParty->vPosition.x);
                        dist_Y = abs(pActors[monster_id].vPosition.y - pParty->vPosition.y);
                        dist_Z = abs(pActors[monster_id].vPosition.z - pParty->vPosition.z);
                        int count = int_get_vector_length(dist_X, dist_Y, dist_Z);
                        if ((double)count <= 307.2) {
                            InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                            pSpellSprite.uSectorID = 0;
                            pSpellSprite.field_60_distance_related_prolly_lod = 0;
                            pSpellSprite.uFacing = 0;
                            pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                            pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                            pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z - (int)((double)pActors[monster_id].uActorHeight * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, spell_targeted_at);
                            Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                        } else {
                            SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                            pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                            continue;
                        }
                    } else {
                        // Previous behavoiur - decrease mana but do not play sound if
                        // spell is targeted wrong
                        // TODO: revisit, maybe we want to properly play sound and display a message here?
                        pPlayer->SpendMana(uRequiredMana);
                        pCastSpell->uSpellID = 0;
                        continue;
                    }
                    break;
                }

                case SPELL_AIR_SHIELD:
                case SPELL_EARTH_STONESKIN:
                case SPELL_SPIRIT_HEROISM:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 900 * (spell_level + 4);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 3600 * (spell_level + 1);
                            break;
                        default:
                            assert(false);
                    }
                    switch (pCastSpell->uSpellID) {
                        case SPELL_AIR_SHIELD:
                            amount = 0;
                            buff_resist = PARTY_BUFF_SHIELD;
                            break;
                        case SPELL_EARTH_STONESKIN:
                            amount = spell_level + 5;
                            buff_resist = PARTY_BUFF_STONE_SKIN;
                            break;
                        case SPELL_SPIRIT_HEROISM:
                            amount = spell_level + 5;
                            buff_resist = PARTY_BUFF_HEROISM;
                            break;
                        default:
                            assert(false);
                            continue;
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[buff_resist].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_FIRE_IMMOLATION:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spellduration = 600 * spell_level;
                    } else {
                        spellduration = 60 * spell_level;
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(spellduration)),
                            spell_mastery, spell_level, 0, 0);
                    break;
                }

                case SPELL_FIRE_METEOR_SHOWER:
                {
                    if (spell_mastery < PLAYER_SKILL_MASTERY_MASTER) {
                        pCastSpell->uSpellID = 0;
                        continue;
                    }

                    int meteor_num;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        meteor_num = 20;
                    } else {
                        meteor_num = 16;
                    }
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        SpellFailed(pCastSpell, LSTR_CANT_METEOR_SHOWER_INDOORS);
                        continue;
                    }
                    obj_type = PID_TYPE(spell_targeted_at);
                    monster_id = PID_ID(spell_targeted_at);
                    if (obj_type == OBJECT_Actor) {  // quick cast can specify target
                        dist_X = pActors[monster_id].vPosition.x;
                        dist_Y = pActors[monster_id].vPosition.y;
                        dist_Z = pActors[monster_id].vPosition.z;
                    } else {
                        dist_X = pParty->vPosition.x + 2048 * pCamera3D->fRotationZCosine;
                        dist_Y = pParty->vPosition.y + 2048 * pCamera3D->fRotationZSine;
                        dist_Z = pParty->vPosition.z;
                    }
                    uint64_t k = 0;
                    int j = 0;
                    if (meteor_num > 0) {
                        int target_pid = obj_type == OBJECT_Actor ? spell_targeted_at : 0;
                        for (; meteor_num; meteor_num--) {
                            uint32_t yaw, pitch;
                            spell_targeted_at = grng->Random(1000);
                            if (sqrt(((double)spell_targeted_at - 2500) *
                                        ((double)spell_targeted_at - 2500) + j * j + k * k) <= 1.0) {
                                yaw = 0;
                                pitch = 0;
                            } else {
                                pitch = TrigLUT.Atan2((int64_t)sqrt((float)(j * j + k * k)),
                                        (double)spell_targeted_at - 2500);
                                yaw = TrigLUT.Atan2(j, k);
                            }
                            InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                            pSpellSprite.vPosition.x = dist_X;
                            pSpellSprite.vPosition.y = dist_Y;
                            pSpellSprite.vPosition.z = spell_targeted_at + dist_Z;
                            pSpellSprite.uSectorID = 0;
                            pSpellSprite.spell_target_pid = target_pid;
                            pSpellSprite.field_60_distance_related_prolly_lod =
                                stru_50C198._427546(spell_targeted_at + 2500);
                            pSpellSprite.uFacing = yaw;
                            if (pParty->bTurnBasedModeOn) {
                                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                            }
                            if (pSpellSprite.Create(yaw, pitch, pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed, 0) != -1 && pParty->bTurnBasedModeOn) {
                                ++pTurnEngine->pending_actions;
                            }
                            j = grng->Random(1024) - 512;
                            k = grng->Random(1024) - 512;
                        }
                    }
                    break;
                }

                case SPELL_FIRE_INFERNO:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                        SpellFailed(pCastSpell, LSTR_CANT_INFERNO_OUTDOORS);
                        continue;
                    }
                    int mon_num = render->GetActorsInViewport(4096);
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (uint i = 0; i < mon_num; i++) {
                        pSpellSprite.vPosition.x = pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.z -
                            (unsigned int)(int64_t)((double)pActors
                                    [_50BF30_actors_in_viewport_ids
                                    [i]].uActorHeight * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, _50BF30_actors_in_viewport_ids[i]);
                        Actor::DamageMonsterFromParty(
                                PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                                _50BF30_actors_in_viewport_ids[i], &spell_velocity);
                        spell_fx_renderer->RenderAsSprite(&pSpellSprite);
                        spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.OrangeyRed.C32(), 0x40);
                    }
                    break;
                }

                case SPELL_AIR_WIZARD_EYE:
                {
                    spellduration = 3600 * spell_level;
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        spell_overlay_id =
                            pOtherOverlayList->_4418B1(2000, pl_id + 100, 0, 65536);
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, 0, spell_overlay_id, 0);
                    break;
                }

                case SPELL_AIR_FEATHER_FALL:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 300 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 3600 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        pOtherOverlayList->_4418B1(2010, pl_id + 100, 0, 65536);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);

                    pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_AIR_SPARKS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 9;
                            break;
                        default:
                            assert(false);
                    }
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_spray_angle_start = ONE_THIRD_PI / -2;
                    spell_spray_angle_end = ONE_THIRD_PI / 2;
                    while (spell_spray_angle_start <= spell_spray_angle_end) {
                        pSpellSprite.uSpriteFrameID = grng->Random(64); // TODO(captainurist): this doesn't look like a frame id initialization
                        pSpellSprite.uFacing = spell_spray_angle_start + (short)target_direction.uYawAngle;
                        if (pSpellSprite.Create(
                                    (int16_t)(spell_spray_angle_start + (short)target_direction.uYawAngle),
                                    target_direction.uPitchAngle,
                                    pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                                    pCastSpell->uPlayerID + 1) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        spell_spray_angle_start += ONE_THIRD_PI / (amount - 1);
                    }
                    break;
                }

                case SPELL_AIR_JUMP:
                {
                    if (pParty->IsAirborne()) {
                        SpellFailed(pCastSpell, LSTR_CANT_JUMP_AIRBORNE);
                        continue;
                    }
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        pOtherOverlayList->_4418B1(2040, pl_id + 100, 0, 65536);
                    }
                    pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                    pParty->uFallSpeed = 1000;
                    break;
                }

                case SPELL_AIR_INVISIBILITY:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 600 * spell_level;
                            amount = spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 600 * spell_level;
                            amount = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 600 * spell_level;
                            amount = 3 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 3600 * spell_level;
                            amount = 4 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    if (pParty->GetRedOrYellowAlert()) {
                        SpellFailed(pCastSpell, LSTR_HOSTILE_CREATURES_NEARBY);
                        continue;
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_AIR_FLY:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        SpellFailed(pCastSpell, LSTR_HOSTILE_CREATURES_NEARBY);
                        continue;
                    }
                    if (!pPlayers[pCastSpell->uPlayerID + 1]->GetMaxMana() && !engine->config->debug.AllMagic.Get()) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    if (spell_mastery < PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 1;
                    } else {
                        amount = 0;
                    }
                    for (uint pl_id = 0; pl_id < 4; pl_id++)
                        pOtherOverlayList->_4418B1(2090, pl_id + 100, 0, 65536);
                    spell_overlay_id = pOtherOverlayList->_4418B1(10008, 203, 0, 65536);
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(3600 * spell_level)),
                            spell_mastery, amount, spell_overlay_id,
                            pCastSpell->uPlayerID + 1);
                    break;
                }

                case SPELL_AIR_STARBURST:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        SpellFailed(pCastSpell, LSTR_CANT_STARBURST_INDOORS);
                        continue;
                    }
                    obj_type = PID_TYPE(spell_targeted_at);
                    monster_id = PID_ID(spell_targeted_at);
                    if (obj_type == OBJECT_Actor) {
                        dist_X = pActors[monster_id].vPosition.x;
                        dist_Y = pActors[monster_id].vPosition.y;
                        dist_Z = pActors[monster_id].vPosition.z;
                    } else {
                        dist_X = pParty->vPosition.x + 2048 * pCamera3D->fRotationZCosine;
                        dist_Y = pParty->vPosition.y + 2048 * pCamera3D->fRotationZSine;
                        dist_Z = pParty->vPosition.z;
                    }
                    uint64_t k = 0;
                    int j = 0;
                    int target_pid = obj_type == OBJECT_Actor ? spell_targeted_at : 0;
                    uint32_t yaw, pitch;
                    for (uint star_num = 20; star_num; star_num--) {
                        spell_targeted_at = grng->Random(1000);
                        if (sqrt(((double)spell_targeted_at + (double)dist_Z -
                                        (double)(dist_Z + 2500)) *
                                    ((double)spell_targeted_at + (double)dist_Z -
                                     (double)(dist_Z + 2500)) +
                                    j * j + k * k) <= 1.0) {
                            yaw = 0;
                            pitch = 0;
                        } else {
                            pitch = TrigLUT.Atan2((int64_t)sqrt((float)(j * j + k * k)),
                                    ((double)spell_targeted_at + (double)dist_Z -
                                     (double)(dist_Z + 2500)));
                            yaw = TrigLUT.Atan2(j, k);
                        }
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = dist_X;
                        pSpellSprite.vPosition.y = dist_Y;
                        pSpellSprite.vPosition.z = (int)(spell_targeted_at + (dist_Z + 2500));
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.spell_target_pid = target_pid;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            stru_50C198._427546(spell_targeted_at + 2500);
                        pSpellSprite.uFacing = yaw;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        if (pSpellSprite.Create(yaw, pitch,
                                    pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed, 0) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        j = grng->Random(1024) - 512;
                        k = grng->Random(1024) - 512;
                    }
                    break;
                }

                case SPELL_WATER_AWAKEN:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 180 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    for (int i = 0; i < 4; i++) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            if (pParty->pPlayers[i].conditions.Has(Condition_Sleep)) {
                                pParty->pPlayers[i].conditions.Reset(Condition_Sleep);
                                pParty->pPlayers[i].PlaySound(SPEECH_Awaken, 0);
                            }
                        } else {
                            if (pParty->pPlayers[i]
                                    .DiscardConditionIfLastsLongerThan(
                                        Condition_Sleep,
                                        GameTime(pParty->GetPlayingTime() - GameTime::FromSeconds(amount))))
                                pParty->pPlayers[i].PlaySound(SPEECH_Awaken, 0);
                        }
                    }
                    break;
                }

                case SPELL_WATER_POISON_SPRAY:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 7;
                            break;
                        default:
                            assert(false);
                    }
                    if (amount == 1) {
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                        if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                    } else {
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        spell_spray_angle_start = ONE_THIRD_PI / -2;
                        spell_spray_angle_end = ONE_THIRD_PI / 2;
                        if (spell_spray_angle_start <= spell_spray_angle_end) {
                            do {
                                pSpellSprite.uFacing = spell_spray_angle_start +
                                    target_direction.uYawAngle;
                                if (pSpellSprite.Create(
                                            pSpellSprite.uFacing,
                                            target_direction.uPitchAngle,
                                            pObjectList
                                            ->pObjects[pSpellSprite.uObjectDescID]
                                            .uSpeed,
                                            pCastSpell->uPlayerID + 1) != -1 &&
                                        pParty->bTurnBasedModeOn) {
                                    ++pTurnEngine->pending_actions;
                                }
                                spell_spray_angle_start += ONE_THIRD_PI / (amount - 1);
                            } while (spell_spray_angle_start <=
                                    spell_spray_angle_end);
                        }
                    }
                    break;
                }

                case SPELL_WATER_WATER_WALK:
                {
                    if (!pPlayers[pCastSpell->uPlayerID + 1]->GetMaxMana()) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:  // break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 3600 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    spell_overlay_id = pOtherOverlayList->_4418B1(10005, 201, 0, 65536);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, spell_overlay_id,
                            pCastSpell->uPlayerID + 1);
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags = 1;
                    }
                    break;
                }

                case SPELL_WATER_RECHARGE_ITEM:
                {
                    ItemGen *item = &pParty->pPlayers[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    if (item->GetItemEquipType() != EQUIP_WAND || item->IsBroken()) {
                        _50C9D0_AfterEnchClickEventId = 113;
                        _50C9D4_AfterEnchClickEventSecondParam = 0;
                        _50C9D8_AfterEnchClickEventTimeout = 1;
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }

                    double spell_recharge_factor;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE || spell_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
                        spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.5;  // 50 %
                    } else if (spell_mastery == PLAYER_SKILL_MASTERY_MASTER) {
                        spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.69999999;  // 30 %
                    } else if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.80000001;  // 20 %
                    } else {
                        spell_recharge_factor = 0.0;
                    }

                    if (spell_recharge_factor > 1.0) {
                        spell_recharge_factor = 1.0;
                    }

                    int uNewCharges = item->uMaxCharges * spell_recharge_factor;
                    item->uMaxCharges = uNewCharges;
                    item->uNumCharges = uNewCharges;
                    if (uNewCharges <= 0) {
                        _50C9D0_AfterEnchClickEventId = 113;
                        _50C9D4_AfterEnchClickEventSecondParam = 0;
                        _50C9D8_AfterEnchClickEventTimeout = 1;
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }

                    item->uAttributes |= ITEM_AURA_EFFECT_GREEN;
                    _50C9A8_item_enchantment_timer = 256;
                    break;
                }

                case SPELL_WATER_ENCHANT_ITEM:
                {
                    uRequiredMana = 0;
                    amount = 10 * spell_level;
                    bool item_not_broken = true;
                    bool spell_failed = true;
                    int rnd = grng->Random(100);
                    pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID_2];
                    ItemGen *spell_item_to_enchant = &pPlayer->pInventoryItemList[pCastSpell->spell_target_pid];
                    ITEM_EQUIP_TYPE this_equip_type = pItemTable->pItems[spell_item_to_enchant->uItemID].uEquipType;

                    // refs
                    // https://www.gog.com/forum/might_and_magic_series/a_little_enchant_item_testing_in_mm7
                    // http://www.pottsland.com/mm6/enchant.shtml
                    // also see STDITEMS.tx and SPCITEMS.txt in Events.lod

                    if ((spell_mastery == PLAYER_SKILL_MASTERY_NOVICE || spell_mastery == PLAYER_SKILL_MASTERY_EXPERT)) {
                        __debugbreak(); // SPELL_WATER_ENCHANT_ITEM is a master level spell
                    }

                    if ((spell_mastery == PLAYER_SKILL_MASTERY_MASTER || spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) &&
                            IsRegular(spell_item_to_enchant->uItemID) &&
                            spell_item_to_enchant->special_enchantment == ITEM_ENCHANTMENT_NULL &&
                            spell_item_to_enchant->uEnchantmentType == 0 &&
                            spell_item_to_enchant->m_enchantmentStrength == 0 &&
                            !spell_item_to_enchant->IsBroken()) {
                        // break items with low value
                        if ((spell_item_to_enchant->GetValue() < 450 && !IsWeapon(this_equip_type)) ||  // not weapons
                                (spell_item_to_enchant->GetValue() < 250 && IsWeapon(this_equip_type))) {  // weapons
                            if (!(spell_item_to_enchant->uAttributes & ITEM_HARDENED)) {
                                spell_item_to_enchant->SetBroken();
                            }
                            item_not_broken = false;
                        } else {
                            // random item break
                            if (rnd >= 10 * spell_level) {  // 10% chance of success per spell level
                                if (!(spell_item_to_enchant->uAttributes & ITEM_HARDENED)) {
                                    spell_item_to_enchant->SetBroken();
                                }
                            } else {
                                // Weapons are limited to special enchantments, but all other types can have either
                                if (rnd < 80 && IsPassiveEquipment(this_equip_type)) { // chance to roll standard enchantment on non-weapons
                                    int ench_found = 0;
                                    int to_item_apply_sum = 0;
                                    int ench_array[100] = { 0 };

                                    // finds how many possible enchaments and adds up to item apply values
                                    // if (pItemTable->pEnchantments_count > 0) {
                                    for (int norm_ench_loop = 0; norm_ench_loop < 24; ++norm_ench_loop) {
                                        char* this_bon_state = pItemTable->pEnchantments[norm_ench_loop].pBonusStat;
                                        if (this_bon_state != NULL && (this_bon_state[0] != '\0')) {
                                            int this_to_apply = pItemTable->pEnchantments[norm_ench_loop].to_item[this_equip_type];
                                            to_item_apply_sum += this_to_apply;
                                            if (this_to_apply) {
                                                ench_array[ench_found] = norm_ench_loop;
                                                ench_found++;
                                            }
                                        }
                                    }
                                    // }

                                    // pick a random ench
                                    int item_apply_rand = grng->Random(to_item_apply_sum);
                                    int target_item_apply_rand = item_apply_rand + 1;
                                    int current_item_apply_sum = 0;
                                    int step = 0;

                                    // step through until we hit that ench
                                    for (step = 0; step < ench_found; step++) {
                                        current_item_apply_sum += pItemTable->pEnchantments[ench_array[step]].to_item[this_equip_type];
                                        if (current_item_apply_sum >= target_item_apply_rand) {
                                            break;
                                        }
                                    }

                                    // assign ench and power
                                    spell_item_to_enchant->uEnchantmentType = (ITEM_ENCHANTMENT)ench_array[step];

                                    int ench_power = 0;
                                    // master 3-8  - guess work needs checking
                                    if (spell_mastery == PLAYER_SKILL_MASTERY_MASTER) ench_power = grng->Random(6) + 3;
                                    // gm 6-12   - guess work needs checking
                                    if (spell_mastery== PLAYER_SKILL_MASTERY_GRANDMASTER) ench_power = grng->Random(7) + 6;

                                    spell_item_to_enchant->m_enchantmentStrength = ench_power;
                                    spell_item_to_enchant->uAttributes |= ITEM_AURA_EFFECT_BLUE;
                                    _50C9A8_item_enchantment_timer = 256;
                                    spell_failed = false;
                                } else { // weapons or we won the lottery for special enchantment
                                    int ench_found = 0;
                                    int to_item_apply_sum = 0;
                                    int ench_array[100] = { 0 };

                                    // finds how many possible enchaments and adds up to item apply values
                                    if (pItemTable->pSpecialEnchantments_count > 0) {
                                        for (ITEM_ENCHANTMENT spec_ench_loop : pItemTable->pSpecialEnchantments.indices()) {
                                            char *this_bon_state = pItemTable->pSpecialEnchantments[spec_ench_loop].pBonusStatement;
                                            if (this_bon_state != NULL && (this_bon_state[0] != '\0')) {
                                                if (pItemTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel == 3) {
                                                    continue;
                                                }
                                                if (spell_mastery == PLAYER_SKILL_MASTERY_MASTER && (pItemTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel != 0)) {
                                                    continue;
                                                }
                                                int this_to_apply = pItemTable->pSpecialEnchantments[spec_ench_loop].to_item_apply[this_equip_type];
                                                to_item_apply_sum += this_to_apply;
                                                if (this_to_apply) {
                                                    ench_array[ench_found] = spec_ench_loop;
                                                    ench_found++;
                                                }
                                            }
                                        }
                                    }

                                    // pick a random ench
                                    int item_apply_rand = grng->Random(to_item_apply_sum);
                                    int target_item_apply_rand = item_apply_rand + 1;
                                    int current_item_apply_sum = 0;
                                    int step = 0;

                                    // step through until we hit that ench
                                    for (step = 0; step < ench_found; step++) {
                                        current_item_apply_sum += pItemTable->pSpecialEnchantments[(ITEM_ENCHANTMENT)ench_array[step]].to_item_apply[this_equip_type];
                                        if (current_item_apply_sum >= target_item_apply_rand) {
                                            break;
                                        }
                                    }

                                    // set item ench
                                    spell_item_to_enchant->special_enchantment = (ITEM_ENCHANTMENT)ench_array[step];
                                    spell_item_to_enchant->uAttributes |= ITEM_AURA_EFFECT_BLUE;
                                    _50C9A8_item_enchantment_timer = 256;
                                    spell_failed = false;
                                }
                            }
                        }
                    }

                    if (spell_failed) {
                        SpellFailed(pCastSpell, item_not_broken ? LSTR_SPELL_FAILED : LSTR_ITEM_TOO_LAME);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_SpellFailed, 0);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    break;
                }

                case SPELL_EARTH_STONE_TO_FLESH:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            break;
                        default:
                            assert(false);
                    }
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Petrified)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Petrified);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Petrified,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                    }
                    break;
                }

                case SPELL_EARTH_ROCK_BLAST:
                {
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = pParty->sRotationZ;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, pParty->sRotationY, spell_speed, pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_EARTH_DEATH_BLOSSOM:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    pSpellSprite.uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition.x = pParty->vPosition.x;
                    pSpellSprite.vPosition.y = pParty->vPosition.y;
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.vPosition.z = pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.uFacing = (short)pParty->sRotationZ;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, TrigLUT.uIntegerHalfPi / 2, spell_speed, 0) != -1 && pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_SPIRIT_DETECT_LIFE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 1800 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 3600 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(amount)),
                            spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_SPIRIT_FATE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 6 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    // LODWORD(spellduration) = 300;
                    if (pCastSpell->spell_target_pid == 0) {
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .pPlayerBuffs[PLAYER_BUFF_FATE]
                            .Apply(
                                    GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(5)),
                                    spell_mastery, amount, 0, 0);
                    } else if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        monster_id = PID_ID(pCastSpell->spell_target_pid);
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_FATE].Apply(
                                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(5)),
                                spell_mastery, amount, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer
                            ->_4A7E89_sparkles_on_actor_after_it_casts_buff(
                                    &pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_SPIRIT_REMOVE_CURSE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Cursed)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Cursed);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Cursed,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Cursed)) {
                            spell_fx_renderer->SetPlayerBuffAnim(
                                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_PRESERVATION:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spellduration = 900 * (spell_level + 4);
                    } else {
                        spellduration = 300 * (spell_level + 12);
                    }
                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE || spell_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                            .Apply(GameTime(pParty->GetPlayingTime() +
                                        GameTime::FromSeconds(spellduration)),
                                    spell_mastery, 0, 0, 0);
                    } else {
                        for (uint pl_id = 0; pl_id < 4; pl_id++) {
                            spell_fx_renderer->SetPlayerBuffAnim(
                                    pCastSpell->uSpellID, pl_id);
                            pParty->pPlayers[pl_id]
                                .pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                                .Apply(GameTime(pParty->GetPlayingTime() +
                                            GameTime::FromSeconds(spellduration)),
                                        spell_mastery, 0, 0, 0);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_TURN_UNDEAD:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE || spell_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
                        spellduration = 60 * (spell_level + 3);
                    } else {
                        spellduration = 300 * spell_level + 180;
                    }
                    int mon_num = render->GetActorsInViewport(4096);
                    spell_fx_renderer
                        ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.White.C32(),
                                192);
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_SPIRIT_TURN_UNDEAD_1;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (spell_targeted_at = 0; spell_targeted_at < mon_num;
                            ++spell_targeted_at) {
                        if (MonsterStats::BelongsToSupertype(
                                    pActors[_50BF30_actors_in_viewport_ids
                                    [spell_targeted_at]]
                                    .pMonsterInfo.uID,
                                    MONSTER_SUPERTYPE_UNDEAD)) {
                            pSpellSprite.vPosition.x =
                                pActors[_50BF30_actors_in_viewport_ids
                                [spell_targeted_at]]
                                .vPosition.x;
                            pSpellSprite.vPosition.y =
                                pActors[_50BF30_actors_in_viewport_ids
                                [spell_targeted_at]]
                                .vPosition.y;
                            pSpellSprite.vPosition.z =
                                pActors[_50BF30_actors_in_viewport_ids
                                [spell_targeted_at]]
                                .vPosition.z -
                                    pActors[_50BF30_actors_in_viewport_ids
                                    [spell_targeted_at]]
                                    .uActorHeight *
                                        -0.8;
                            pSpellSprite.spell_target_pid = PID(
                                    OBJECT_Actor,
                                    _50BF30_actors_in_viewport_ids[spell_targeted_at]);
                            pSpellSprite.Create(0, 0, 0, 0);
                            pActors
                                [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                                .pActorBuffs[ACTOR_BUFF_AFRAID]
                                    .Apply(GameTime(pParty->GetPlayingTime() +
                                                GameTime::FromSeconds(spellduration)),
                                            spell_mastery, 0, 0, 0);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_RAISE_DEAD:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 0;
                    } else {
                        amount = 86400 * spell_level;
                    }
                    pOtherOverlayList->_4418B1(5080, pCastSpell->uPlayerID_2 + 100, 0, 65536);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Dead)) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Dead);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Unconscious);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Dead, GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Unconscious,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                                Condition_Weak, 0);
                    }
                    break;
                }

                case SPELL_SPIRIT_SHARED_LIFE:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 4 * spell_level;
                    } else {
                        amount = 3 * spell_level;
                    }
                    int active_pl_num = 0;
                    signed int shared_life_count = amount;
                    int mean_life = 0;
                    int pl_array[4] {};
                    for (uint pl_id = 1; pl_id <= 4; pl_id++) {
                        if (!pPlayers[pl_id]->conditions.Has(Condition_Dead) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Petrified) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Eradicated))
                            pl_array[active_pl_num++] = pl_id;
                    }
                    for (uint i = 0; i < active_pl_num; i++)
                        shared_life_count += pPlayers[pl_array[i]]->sHealth;
                    mean_life = (int64_t)((double)shared_life_count /
                            (double)active_pl_num);
                    for (uint i = 0; i < active_pl_num; i++) {
                        pPlayers[pl_array[i]]->sHealth = mean_life;
                        if (pPlayers[pl_array[i]]->sHealth >
                                pPlayers[pl_array[i]]->GetMaxHealth())
                            pPlayers[pl_array[i]]->sHealth =
                                pPlayers[pl_array[i]]->GetMaxHealth();
                        if (pPlayers[pl_array[i]]->sHealth > 0)
                            pPlayers[pl_array[i]]->SetUnconcious(GameTime(0));
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pl_array[i] - 1);
                    }
                    break;
                }

                case SPELL_SPIRIT_RESSURECTION:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 180 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 10800 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 259200 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Eradicated) ||
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Dead)) {
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak))
                            pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_Weak, 0);
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Eradicated);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Dead);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Unconscious);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Eradicated,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Dead,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Unconscious,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                                Condition_Weak, 1);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    break;
                }

                case SPELL_MIND_CURE_PARALYSIS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Paralyzed)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Paralyzed);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Paralyzed, GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                    }
                    break;
                }

                case SPELL_MIND_REMOVE_FEAR:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 180 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Fear)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Fear);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Fear, GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                    }

                    break;
                }

                case SPELL_MIND_TELEPATHY:
                {
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        monster_id = PID_ID(spell_targeted_at);
                        if (!pActors[monster_id].ActorHasItem())
                            pActors[monster_id].SetRandomGoldIfTheresNoItem();
                        int gold_num = 0;
                        if (pActors[monster_id].ActorHasItems[3].uItemID != ITEM_NULL) {
                            if (pItemTable->pItems[pActors[monster_id].ActorHasItems[3].uItemID].uEquipType == EQUIP_GOLD)
                                gold_num = pActors[monster_id].ActorHasItems[3].special_enchantment;
                        }
                        ItemGen item;
                        item.Reset();
                        if (pActors[monster_id].uCarriedItemID != ITEM_NULL) {
                            item.uItemID = pActors[monster_id].uCarriedItemID;
                        } else {
                            for (uint i = 0; i < 4; ++i) {
                                if (pActors[monster_id].ActorHasItems[i].uItemID != ITEM_NULL &&
                                        pItemTable
                                        ->pItems[pActors[monster_id]
                                        .ActorHasItems[i]
                                        .uItemID]
                                        .uEquipType != EQUIP_GOLD) {
                                    memcpy(&item,
                                            &pActors[monster_id].ActorHasItems[i],
                                            sizeof(item));
                                    spell_level = spell_level;
                                }
                            }
                        }
                        if (gold_num > 0) {
                            if (item.uItemID != ITEM_NULL)
                                GameUI_SetStatusBar(StringPrintf(
                                            "(%s), and %d gold",
                                            item.GetDisplayName().c_str(), gold_num));
                            else
                                GameUI_SetStatusBar(StringPrintf(
                                            "%d gold", gold_num));
                        } else {
                            if (item.uItemID != ITEM_NULL) {
                                GameUI_SetStatusBar(StringPrintf(
                                            "(%s)", item.GetDisplayName().c_str()));
                            } else {
                                GameUI_SetStatusBar("nothing");
                            }
                        }

                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].uActorHeight;
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_BERSERK:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 300 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 300 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 3600;
                            break;
                        default:
                            assert(false);
                    }
                    monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        // v730 = 836 * monster_id;
                        if (pActors[monster_id].DoesDmgTypeDoDamage(
                                    DMGT_MIND)) {
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_CHARM]
                                .Reset();
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_ENSLAVED]
                                .Reset();
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_BERSERK]
                                .Apply(GameTime(pParty->GetPlayingTime() +
                                            GameTime::FromSeconds(amount)),
                                        spell_mastery, 0, 0, 0);
                            pActors[monster_id].pMonsterInfo.uHostilityType =
                                MonsterInfo::Hostility_Long;
                        }
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z +
                            pActors[monster_id].uActorHeight;
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_ENSLAVE:
                {
                    amount = 600 * spell_level;
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        monster_id = PID_ID(spell_targeted_at);
                        // v730 = 836 * monster_id;
                        if (MonsterStats::BelongsToSupertype(
                                    pActors[monster_id].pMonsterInfo.uID,
                                    MONSTER_SUPERTYPE_UNDEAD)) {
                            break;
                        }
                        if (pActors[monster_id].DoesDmgTypeDoDamage(
                                    DMGT_MIND)) {
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_BERSERK]
                                .Reset();
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_CHARM]
                                .Reset();
                            pActors[monster_id]
                                .pActorBuffs[ACTOR_BUFF_ENSLAVED]
                                .Apply(GameTime(pParty->GetPlayingTime() +
                                            GameTime::FromSeconds(amount)),
                                        spell_mastery, 0, 0, 0);
                        }
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z + pActors[monster_id].uActorHeight;
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_MASS_FEAR:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 300 * spell_level;
                    } else {
                        amount = 180 * spell_level;
                    }
                    int mon_num = render->GetActorsInViewport(4096);
                    spell_fx_renderer
                        ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Night.C32(),
                                192);
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_MIND_MASS_FEAR_1;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (spell_targeted_at = 0; spell_targeted_at < mon_num;
                            ++spell_targeted_at) {
                        if (MonsterStats::BelongsToSupertype(
                                    pActors[_50BF30_actors_in_viewport_ids
                                    [spell_targeted_at]]
                                    .pMonsterInfo.uID,
                                    MONSTER_SUPERTYPE_UNDEAD)) {
                            break;
                        }
                        pSpellSprite.vPosition.x =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.x;
                        pSpellSprite.vPosition.y =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.y;
                        pSpellSprite.vPosition.z =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.z -
                                (unsigned int)(int64_t)((double)pActors
                                        [_50BF30_actors_in_viewport_ids
                                        [spell_targeted_at]]
                                        .uActorHeight *
                                        -0.8);
                        pSpellSprite.spell_target_pid =
                            PID(OBJECT_Actor,
                                    _50BF30_actors_in_viewport_ids[spell_targeted_at]);
                        pSpellSprite.Create(0, 0, 0, 0);
                        if (pActors
                                [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                                .DoesDmgTypeDoDamage(DMGT_MIND)) {
                            pActors
                                [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                                .pActorBuffs[ACTOR_BUFF_AFRAID]
                                    .Apply(GameTime(pParty->GetPlayingTime() +
                                                GameTime::FromSeconds(amount)),
                                            spell_mastery, 0, 0, 0);
                        }
                    }
                    break;
                }

                case SPELL_MIND_CURE_INSANITY:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 0;
                    } else {
                        amount = 86400 * spell_level;
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Insane)) {
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak))
                            pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_Weak, 0);
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER)
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Insane);
                        else
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Insane,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                                Condition_Weak, 0);
                    }
                    break;
                }

                case SPELL_EARTH_TELEKINESIS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 3 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 4 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    int obj_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Item) {
                        if (pItemTable
                                ->pItems[pSpriteObjects[obj_id]
                                .containing_item.uItemID]
                                .uEquipType == EQUIP_GOLD) {
                            pParty->PartyFindsGold(
                                    pSpriteObjects[obj_id]
                                    .containing_item.special_enchantment,
                                    0);
                            viewparams->bRedrawGameUI = true;
                        } else {
                            GameUI_SetStatusBar(
                                    LSTR_FMT_YOU_FOUND_ITEM,
                                    pItemTable->pItems[pSpriteObjects[obj_id].containing_item.uItemID].pUnidentifiedName
                                    );
                            if (!pParty->AddItemToParty(
                                        &pSpriteObjects[obj_id].containing_item))
                                pParty->SetHoldingItem(
                                        &pSpriteObjects[obj_id].containing_item);
                        }
                        SpriteObject::OnInteraction(obj_id);
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor)
                        pActors[obj_id].LootActor();
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Decoration) {
                        OpenedTelekinesis = true;
                        if (pLevelDecorations[obj_id].uEventID)
                            EventProcessor(pLevelDecorations[obj_id].uEventID,
                                    spell_targeted_at, 1);
                        if (pLevelDecorations[std::to_underlying(pSpriteObjects[obj_id]
                                    .containing_item.uItemID)] // TODO(captainurist): investigate, that's a very weird std::to_underlying call.
                                .IsInteractive()) {
                            activeLevelDecoration = &pLevelDecorations[obj_id];
                            EventProcessor(
                                    stru_5E4C90_MapPersistVars._decor_events
                                    [pLevelDecorations[obj_id]._idx_in_stru123 -
                                    75] +
                                    380,
                                    0, 1);
                            activeLevelDecoration = nullptr;
                        }
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Face) {
                        int event;
                        OpenedTelekinesis = true;
                        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                            event = pIndoor->pFaceExtras[pIndoor->pFaces[obj_id].uFaceExtraID].uEventID;
                        } else {
                            event = pOutdoor->pBModels[spell_targeted_at >> 9].pFaces[obj_id & 0x3F].sCogTriggeredID;
                        }
                        EventProcessor(event, spell_targeted_at, 1);
                    }
                    break;
                }

                case SPELL_BODY_CURE_WEAKNESS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 60 * 3 * spell_level; // skill points * (3 minutes)
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 60 * 60 * spell_level; // skill points * (1 hour)
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 60 * 60 * 24 * spell_level; // skill points * (1 day)
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Weak);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Weak, GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                    }
                    break;
                }

                case SPELL_BODY_FIRST_AID:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 2 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 4 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 5 * spell_level + 5;
                            break;
                        default:
                            assert(false);
                    }
                    if (!pCastSpell->spell_target_pid) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2].Heal(amount);
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        monster_id = PID_ID(pCastSpell->spell_target_pid);
                        if (pActors[monster_id].uAIState != Dead &&
                                pActors[monster_id].uAIState != Dying &&
                                pActors[monster_id].uAIState != Disabled &&
                                pActors[monster_id].uAIState != Removed) {
                            pActors[monster_id].sCurrentHP += amount;
                            if (pActors[monster_id].sCurrentHP >
                                    pActors[monster_id].pMonsterInfo.uHP)
                                pActors[monster_id].sCurrentHP =
                                    pActors[monster_id].pMonsterInfo.uHP;
                        }
                    }
                    break;
                }

                case SPELL_BODY_CURE_POISON:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3600 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 86400 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 0;
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Weak) ||
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Medium) ||
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Severe)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Weak);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Medium);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Severe);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Poison_Weak,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Poison_Medium,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(
                                        Condition_Poison_Severe,
                                        GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));
                        }
                    }
                    break;
                }

                case SPELL_BODY_PROTECTION_FROM_MAGIC:
                {
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(3600 * spell_level)),
                            spell_mastery, spell_level, 0, 0);
                    break;
                }

                case SPELL_BODY_HAMMERHANDS:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                        for (uint pl_id = 0; pl_id < 4; pl_id++) {
                            pParty->pPlayers[pl_id]
                                .pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                                .Apply(
                                        GameTime(pParty->GetPlayingTime() +
                                            GameTime::FromSeconds(3600 * spell_level)),
                                        spell_mastery, spell_level, spell_level, 0);
                        }
                    } else {
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                    GameTime::FromSeconds(3600 * spell_level)),
                                spell_mastery, spell_level, spell_level, 0);
                    }
                    break;
                }

                case SPELL_BODY_POWER_CURE:
                {
                    for (uint pl_id = 0; pl_id < 4; ++pl_id) {
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pl_id);
                        pParty->pPlayers[pl_id].Heal(5 * spell_level + 10);
                    }
                    break;
                }

                case SPELL_LIGHT_DISPEL_MAGIC:
                {
                    sRecoveryTime -= spell_level;
                    spell_fx_renderer
                        ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.VibrantGreen.C32(),
                                192);
                    int mon_num = render->GetActorsInViewport(4096);
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_DISPEL_MAGIC_1;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (spell_targeted_at = 0; spell_targeted_at < mon_num;
                            ++spell_targeted_at) {
                        pSpellSprite.vPosition.x =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.x;
                        pSpellSprite.vPosition.y =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.y;
                        pSpellSprite.vPosition.z =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.z -
                                (int)((double)pActors[_50BF30_actors_in_viewport_ids
                                        [spell_targeted_at]]
                                        .uActorHeight *
                                        -0.8);
                        pSpellSprite.spell_target_pid =
                            PID(OBJECT_Actor,
                                    _50BF30_actors_in_viewport_ids[spell_targeted_at]);
                        Actor::DamageMonsterFromParty(
                                PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                                _50BF30_actors_in_viewport_ids[spell_targeted_at],
                                &spell_velocity);
                    }
                    for (spell_targeted_at = 0; spell_targeted_at < mon_num;
                            ++spell_targeted_at) {
                        pSpellSprite.vPosition.x =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.x;
                        pSpellSprite.vPosition.y =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.y;
                        pSpellSprite.vPosition.z =
                            pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                            .vPosition.z -
                                (unsigned int)(int64_t)((double)pActors
                                        [_50BF30_actors_in_viewport_ids
                                        [spell_targeted_at]]
                                        .uActorHeight *
                                        -0.8);
                        pSpellSprite.spell_target_pid =
                            PID(OBJECT_Actor,
                                    _50BF30_actors_in_viewport_ids[spell_targeted_at]);
                        pSpellSprite.Create(0, 0, 0, 0);
                        for (SpellBuff &buff : pActors[_50BF30_actors_in_viewport_ids[spell_targeted_at]].pActorBuffs)
                            buff.Reset();
                    }
                    break;
                }

                case SPELL_LIGHT_SUMMON_ELEMENTAL:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 300 * spell_level;
                            amount = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 300 * spell_level;
                            amount = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 900 * spell_level;
                            amount = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 900 * spell_level;
                            amount = 5;
                            break;
                        default:
                            assert(false);
                    }
                    int mon_num = 0;
                    for (uint monster_id = 0; monster_id < pActors.size(); monster_id++) {
                        if (pActors[monster_id].uAIState != Dead &&
                                pActors[monster_id].uAIState != Removed &&
                                pActors[monster_id].uAIState != Disabled &&
                                PID(OBJECT_Player, pCastSpell->uPlayerID) == pActors[monster_id].uSummonerID)
                            ++mon_num;
                    }
                    if (mon_num >= amount) {
                        SpellFailed(pCastSpell, LSTR_SUMMONS_LIMIT_REACHED);
                        continue;
                    }
                    Spawn_Light_Elemental(pCastSpell->uPlayerID, spell_mastery, spellduration);
                    break;
                }

                case SPELL_LIGHT_DAY_OF_THE_GODS:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 10800 * spell_level;
                            amount = 3 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 10800 * spell_level;
                            amount = 3 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 14400 * spell_level;
                            amount = 4 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 18000 * spell_level;
                            amount = 5 * spell_level + 10;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    break;
                }

                case SPELL_LIGHT_PRISMATIC_LIGHT:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                        SpellFailed(pCastSpell, LSTR_CANT_PRISMATIC_OUTDOORS);
                        continue;
                    }
                    int mon_num = render->GetActorsInViewport(4096);
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT_1;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (uint monster_id = 0; monster_id < mon_num; monster_id++) {
                        pSpellSprite.vPosition.x =
                            pActors[_50BF30_actors_in_viewport_ids[monster_id]].vPosition.x;
                        pSpellSprite.vPosition.y =
                            pActors[_50BF30_actors_in_viewport_ids[monster_id]].vPosition.y;
                        pSpellSprite.vPosition.z =
                            pActors[_50BF30_actors_in_viewport_ids[monster_id]].vPosition.z -
                            pActors[_50BF30_actors_in_viewport_ids[monster_id]].uActorHeight * -0.8;
                        pSpellSprite.spell_target_pid =
                            PID(OBJECT_Actor, _50BF30_actors_in_viewport_ids[monster_id]);
                        Actor::DamageMonsterFromParty(
                                PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                                _50BF30_actors_in_viewport_ids[monster_id], &spell_velocity);
                    }
                    spell_fx_renderer->_4A8BFC_prismatic_light();
                    break;
                }

                case SPELL_LIGHT_DAY_OF_PROTECTION:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 14400 * spell_level;
                            amount = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 14400 * spell_level;
                            amount = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 14400 * spell_level;
                            amount = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 18000 * spell_level;
                            amount = 5 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_BODY].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_MIND].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_FIRE].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_WATER].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_AIR].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_EARTH].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, amount, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, spell_level + 5, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                            spell_mastery, spell_level + 5, 0, 0);
                    break;
                }

                case SPELL_LIGHT_HOUR_OF_POWER:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 4;
                            amount = 4;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 4;
                            amount = 4;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 12;
                            amount = 12;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 20;
                            amount = 15;
                            break;
                        default:
                            assert(false);
                    }
                    bool player_weak = false;
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                        pParty->pPlayers[pl_id]
                            .pPlayerBuffs[PLAYER_BUFF_BLESS]
                            .Apply(GameTime(pParty->GetPlayingTime() +
                                        GameTime::FromSeconds(300 * amount * spell_level + 60)),
                                    spell_mastery, spell_level + 5, 0, 0);
                        if (pParty->pPlayers[pl_id].conditions.Has(Condition_Weak)) {
                            player_weak = true;
                        }
                    }

                    pParty->pPartyBuffs[PARTY_BUFF_HEROISM].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(300 * amount * spell_level + 60)),
                            spell_mastery, spell_level + 5, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_SHIELD].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(300 * amount * spell_level + 60)),
                            spell_mastery, 0, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_STONE_SKIN].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(300 * amount * spell_level + 60)),
                            spell_mastery, spell_level + 5, 0, 0);
                    if (!player_weak) {
                        pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(
                                GameTime(pParty->GetPlayingTime() +
                                    GameTime::FromSeconds(
                                        60 * (spell_level * spellduration + 60))),
                                spell_mastery, spell_level + 5, 0, 0);
                    }
                    break;
                }

                case SPELL_LIGHT_DIVINE_INTERVENTION:
                {
                    if (pPlayer->uNumDivineInterventionCastsThisDay >= 3) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        pParty->pPlayers[pl_id].conditions.ResetAll();
                        pParty->pPlayers[pl_id].sHealth = pParty->pPlayers[pl_id].GetMaxHealth();
                        pParty->pPlayers[pl_id].sMana = pParty->pPlayers[pl_id].GetMaxMana();
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pl_id);
                    }
                    if (pPlayer->sAgeModifier + 10 >= 120) {
                        pPlayer->sAgeModifier = 120;
                    } else {
                        pPlayer->sAgeModifier = pPlayer->sAgeModifier + 10;
                    }
                    sRecoveryTime += -5 * spell_level;
                    ++pPlayer->uNumDivineInterventionCastsThisDay;
                    break;
                }

                case SPELL_DARK_REANIMATE:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 3 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 5 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    if (!pCastSpell->spell_target_pid) {
                        spell_fx_renderer->SetPlayerBuffAnim(
                                pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Dead)) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(Condition_Zombie, 1);
                            GameUI_ReloadPlayerPortraits(pCastSpell->uPlayerID_2, (pParty->pPlayers[pCastSpell->uPlayerID_2].GetSexByVoice() != 0) + 23);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
                            // TODO: why call SetCondition and then conditions.Set?
                        }
                        break;
                    }
                    monster_id = PID_ID(pCastSpell->spell_target_pid);
                    if (monster_id == -1) {
                        SpellFailed(pCastSpell, LSTR_NO_VALID_SPELL_TARGET);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    if (pActors[monster_id].sCurrentHP > 0 || pActors[monster_id].uAIState != Dead && pActors[monster_id].uAIState != Dying) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_DARK_REANIMATE_1;
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z -
                        (int)((double)pActors[monster_id].uActorHeight * -0.8);
                    pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                    pSpellSprite.Create(0, 0, 0, 0);
                    if (pActors[monster_id].pMonsterInfo.uLevel > amount) {
                        break;
                    }
                    Actor::Resurrect(monster_id);
                    pActors[monster_id].pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
                    pActors[monster_id].pMonsterInfo.uTreasureDropChance = 0;
                    pActors[monster_id].pMonsterInfo.uTreasureDiceRolls = 0;
                    pActors[monster_id].pMonsterInfo.uTreasureDiceSides = 0;
                    pActors[monster_id].pMonsterInfo.uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID;
                    pActors[monster_id].pMonsterInfo.uTreasureType = 0;
                    pActors[monster_id].uAlly = 9999;
                    pActors[monster_id].ResetAggressor();  // ~0x80000
                    pActors[monster_id].uGroup = 0;
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Reset();
                    if (pActors[monster_id].sCurrentHP > 10 * amount) {
                        pActors[monster_id].sCurrentHP = 10 * amount;
                    }
                    break;
                }

                case SPELL_DARK_SHARPMETAL:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            amount = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            amount = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            amount = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            amount = 9;
                            break;
                        default:
                            assert(false);
                    }
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    // TODO(pskelton): was pParty->uPartyHeight / 2
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    spell_spray_angle_start = ONE_THIRD_PI / -2;
                    spell_spray_angle_end = ONE_THIRD_PI / 2;
                    if (spell_spray_angle_start <= spell_spray_angle_end) {
                        do {
                            pSpellSprite.uFacing = spell_spray_angle_start + target_direction.uYawAngle;
                            if (pSpellSprite.Create(pSpellSprite.uFacing, target_direction.uPitchAngle, pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                                        pCastSpell->uPlayerID + 1) != -1 && pParty->bTurnBasedModeOn) {
                                ++pTurnEngine->pending_actions;
                            }
                            spell_spray_angle_start += ONE_THIRD_PI / (amount - 1);
                        } while (spell_spray_angle_start <= spell_spray_angle_end);
                    }
                    break;
                }

                case SPELL_DARK_CONTROL_UNDEAD:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 180 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 180 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 300 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 29030400;
                            break;
                        default:
                            assert(false);
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        monster_id = PID_ID(spell_targeted_at);
                        if (!MonsterStats::BelongsToSupertype(
                                    pActors[monster_id].pMonsterInfo.uID,
                                    MONSTER_SUPERTYPE_UNDEAD)) {
                            break;
                        }
                        if (!pActors[monster_id].DoesDmgTypeDoDamage(DMGT_DARK)) {
                            SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                            pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                            continue;
                        }
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Apply(
                                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spellduration)),
                                spell_mastery, 0, 0, 0);
                        InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z + pActors[monster_id].uActorHeight;
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_DARK_SACRIFICE:
                {
                    int hired_npc = 0;
                    memset(&achieved_awards, 0, 4000);
                    for (uint npc_id = 0; npc_id < 2; npc_id++) {
                        if (!pParty->pHirelings[npc_id].pName.empty()) {
                            achieved_awards[hired_npc++] = (AwardType)(npc_id + 1);
                        }
                    }

                    if (pCastSpell->uPlayerID_2 != 4 && pCastSpell->uPlayerID_2 != 5 ||
                            achieved_awards[pCastSpell->uPlayerID_2 - 4] <= 0 ||
                            achieved_awards[pCastSpell->uPlayerID_2 - 4] >= 3) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    int hireling_idx = achieved_awards[pCastSpell->uPlayerID_2 - 4] - 1;
                    pParty->pHirelings[hireling_idx].dialogue_1_evt_id = 1;
                    pParty->pHirelings[hireling_idx].dialogue_2_evt_id = 0;
                    pParty->pHirelings[hireling_idx].dialogue_3_evt_id = pIconsFrameTable->GetIcon("spell96")->GetAnimLength();
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        pParty->pPlayers[pl_id].sHealth = pParty->pPlayers[pl_id].GetMaxHealth();
                        pParty->pPlayers[pl_id].sMana = pParty->pPlayers[pl_id].GetMaxMana();
                    }
                    DDM_DLV_Header *ddm_dlv = &pOutdoor->ddm;
                    if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) {
                        ddm_dlv = &pIndoor->dlv;
                    }
                    ddm_dlv->uReputation += 15;
                    if (ddm_dlv->uReputation > 10000) {
                        ddm_dlv->uReputation = 10000;
                    }
                    break;
                }

                case SPELL_DARK_PAIN_REFLECTION:
                {
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spellduration = 300 * (spell_level + 12);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spellduration = 900 * (spell_level + 4);
                            break;
                        default:
                            assert(false);
                    }
                    amount = spell_level + 5;
                    if (spell_mastery != PLAYER_SKILL_MASTERY_MASTER && spell_mastery != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                            .Apply(GameTime(pParty->GetPlayingTime() +
                                        GameTime::FromSeconds(spellduration)),
                                    spell_mastery, amount, 0, 0);
                    } else {
                        for (uint pl_id = 0; pl_id < 4; pl_id++) {
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pl_id);
                            pParty->pPlayers[pl_id]
                                .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                                .Apply(GameTime(pParty->GetPlayingTime() +
                                            GameTime::FromSeconds(spellduration)),
                                        spell_mastery, amount, 0, 0);
                        }
                    }
                    break;
                }

                case SPELL_DARK_SOULDRINKER:
                {
                    int mon_num = render->GetActorsInViewport((int64_t)pCamera3D->GetMouseInfoDepth());
                    InitSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    amount = 0;
                    if (mon_num > 0) {
                        amount = (mon_num * (7 * spell_level + 25));
                        for (uint monster_id = 0; monster_id < mon_num;
                                monster_id++) {
                            pSpellSprite.vPosition.x =
                                pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                                .vPosition.x;
                            pSpellSprite.vPosition.y =
                                pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                                .vPosition.y;
                            pSpellSprite.vPosition.z =
                                pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                                .vPosition.z -
                                (int)((double)pActors[_50BF30_actors_in_viewport_ids
                                        [monster_id]]
                                        .uActorHeight *
                                        -0.8);
                            pSpellSprite.spell_target_pid =
                                PID(OBJECT_Actor,
                                        _50BF30_actors_in_viewport_ids[monster_id]);
                            Actor::DamageMonsterFromParty(
                                    PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                                    _50BF30_actors_in_viewport_ids[monster_id], &spell_velocity);
                        }
                    }
                    int pl_num = 0;
                    int pl_array[4] {};
                    for (uint pl_id = 1; pl_id <= 4; ++pl_id) {
                        if (!pPlayers[pl_id]->conditions.Has(Condition_Sleep) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Paralyzed) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Unconscious) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Dead) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Petrified) &&
                                !pPlayers[pl_id]->conditions.Has(Condition_Eradicated)) {
                            pl_array[pl_num++] = pl_id;
                        }
                    }
                    for (uint j = 0; j < pl_num; j++) {
                        pPlayers[pl_array[j]]->sHealth +=
                            (int64_t)((double)(signed int)amount /
                                    (double)pl_num);
                        if (pPlayers[pl_array[j]]->sHealth > pPlayers[pl_array[j]]->GetMaxHealth())
                            pPlayers[pl_array[j]]->sHealth = pPlayers[pl_array[j]]->GetMaxHealth();
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pl_array[j]);
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Black.C32(), 64);
                    break;
                }

                case SPELL_DARK_ARMAGEDDON:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        SpellFailed(pCastSpell, LSTR_CANT_ARMAGEDDON_INDOORS);
                        continue;
                    }
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        amount = 4;
                    } else {
                        amount = 3;
                    }
                    if (pPlayer->uNumArmageddonCasts >= amount ||
                            pParty->armageddon_timer > 0) {
                        SpellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    pParty->armageddon_timer = 256;
                    pParty->armageddonDamage = spell_level;
                    ++pPlayer->uNumArmageddonCasts;
                    if (pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    for (uint i = 0; i < 50; i++) {
                        int rand_x = grng->Random(4096) - 2048;
                        int rand_y = grng->Random(4096) - 2048;
                        bool bOnWater = false;
                        int terr_height = GetTerrainHeightsAroundParty2(
                                rand_x + pParty->vPosition.x,
                                rand_y + pParty->vPosition.y, &bOnWater, 0);
                        SpriteObject::Drop_Item_At(
                                SPRITE_SPELL_EARTH_ROCK_BLAST,
                                rand_x + pParty->vPosition.x,
                                rand_y + pParty->vPosition.y, terr_height + 16,
                                grng->Random(500) + 500, 1, 0, 0, 0);
                    }
                    break;
                }
                default:
                    pCastSpell->uSpellID = 0;
                    continue;
            }

            spell_sound_flag = true;
            pPlayer->SpendMana(uRequiredMana);
        }

        if (~pCastSpell->uFlags & ON_CAST_NoRecoverySpell) {
            if (sRecoveryTime < 0) {
                sRecoveryTime = 0;
            }

            pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID];  // reset to player who
                                                                 // actually casts spell

            if (pParty->bTurnBasedModeOn) {
                // v645 = sRecoveryTime;
                pParty->pTurnBasedPlayerRecoveryTimes[pCastSpell->uPlayerID] = sRecoveryTime;

                pPlayer->SetRecoveryTime(sRecoveryTime);

                if (!some_active_character) {
                    pTurnEngine->ApplyPlayerAction();
                }
            } else {
                pPlayer->SetRecoveryTime((int64_t)(debug_non_combat_recovery_mul * (double)sRecoveryTime * flt_debugrecmod3));
            }
            pPlayer->PlaySound(SPEECH_CastSpell, 0);
        }
        if (spell_sound_flag) {
            pAudioPlayer->PlaySpellSound(pCastSpell->uSpellID, PID_INVALID);
        }

        pCastSpell->uSpellID = 0;
    }
}

/**
 * @offset 0x00427DA0
 */
size_t PushCastSpellInfo(uint16_t uSpellID, uint16_t uPlayerID,
                         int16_t skill_level, SpellCastFlags uFlags,
                         int spell_sound_id) {
    // uFlags: ON_CAST_*
    for (size_t i = 0; i < CastSpellInfoCount; i++) {
        if (!pCastSpellInfo[i].uSpellID) {
            pCastSpellInfo[i].uSpellID = uSpellID;
            pCastSpellInfo[i].uPlayerID = uPlayerID;
            if (uFlags & ON_CAST_TargetIsParty) pCastSpellInfo[i].uPlayerID_2 = uPlayerID;
            pCastSpellInfo[i].field_6 = 0;
            pCastSpellInfo[i].spell_target_pid = 0;
            pCastSpellInfo[i].uFlags = uFlags;
            pCastSpellInfo[i].forced_spell_skill_level = skill_level;
            pCastSpellInfo[i].sound_id = spell_sound_id;
            return i;
        }
    }
    return -1;
}

/**
 * @offset 0x00427D48
 */
void CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress() {  // reset failed/cancelled spell
    for (size_t i = 0; i < CastSpellInfoCount; i++) {
        if (pCastSpellInfo[i].uSpellID &&
            pCastSpellInfo[i].uFlags & ON_CAST_CastingInProgress) {
            pCastSpellInfo[i].uSpellID = 0;

            if (pGUIWindow_CastTargetedSpell) {
                pGUIWindow_CastTargetedSpell->Release();
                pGUIWindow_CastTargetedSpell = nullptr;
            }
            mouse->SetCursorImage("MICON1");
            GameUI_StatusBar_Update(true);
            _50C9A0_IsEnchantingInProgress = 0;
            back_to_game();
        }
    }
}

/**
 * @offset 0x0042777D
 */
void _42777D_CastSpell_UseWand_ShootArrow(SPELL_TYPE spell,
                                          unsigned int uPlayerID,
                                          PLAYER_SKILL skill_value, SpellCastFlags flags,
                                          int a6) {
    // if (!pParty->bTurnBasedModeOn
    //  || (result = pTurnEngine->field_4, pTurnEngine->field_4 != 1) &&
    //  pTurnEngine->field_4 != 3 )
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage == TE_WAIT ||
            pTurnEngine->turn_stage == TE_MOVEMENT)
            return;
    }

    // spell_pointed_target = a5;
    // v7 = &pParty->pPlayers[uPlayerID];
    assert(uPlayerID < 4);
    Player *player = &pParty->pPlayers[uPlayerID];
    if (!(flags & ON_CAST_TargetIsParty)) {
        switch (spell) {
            case SPELL_SPIRIT_FATE:
            case SPELL_BODY_FIRST_AID:
            case SPELL_DARK_REANIMATE:
                // HIBYTE(spell_pointed_target) = HIBYTE(a5) | 1;
                flags |= ON_CAST_MonsterSparkles;
                break;

            case SPELL_FIRE_FIRE_AURA:
            case SPELL_WATER_RECHARGE_ITEM:
            case SPELL_WATER_ENCHANT_ITEM:
            case SPELL_DARK_VAMPIRIC_WEAPON:
                flags |= ON_CAST_Enchantment;
                break;

            case SPELL_FIRE_FIRE_BOLT:
            case SPELL_FIRE_FIREBALL:
            case SPELL_FIRE_INCINERATE:
            case SPELL_AIR_LIGHNING_BOLT:
            case SPELL_AIR_IMPLOSION:
            case SPELL_WATER_POISON_SPRAY:
            case SPELL_WATER_ICE_BOLT:
            case SPELL_WATER_ACID_BURST:
            case SPELL_WATER_ICE_BLAST:
            case SPELL_EARTH_STUN:
            case SPELL_EARTH_SLOW:
            case SPELL_EARTH_DEADLY_SWARM:
            case SPELL_EARTH_BLADES:
            case SPELL_EARTH_MASS_DISTORTION:
            case SPELL_SPIRIT_SPIRIT_LASH:
            case SPELL_MIND_MIND_BLAST:
            case SPELL_MIND_CHARM:
            case SPELL_MIND_PSYCHIC_SHOCK:
            case SPELL_BODY_HARM:
            case SPELL_BODY_FLYING_FIST:
            case SPELL_LIGHT_LIGHT_BOLT:
            case SPELL_LIGHT_DESTROY_UNDEAD:
            case SPELL_LIGHT_SUNRAY:
            case SPELL_DARK_TOXIC_CLOUD:
            case SPELL_DARK_SHRINKING_RAY:
            case SPELL_DARK_SHARPMETAL:
            case SPELL_DARK_DRAGON_BREATH:
                if (!a6) flags |= ON_CAST_TargetCrosshair;
                break;
            case SPELL_MIND_TELEPATHY:
            case SPELL_MIND_BERSERK:
            case SPELL_MIND_ENSLAVE:
            case SPELL_LIGHT_PARALYZE:
            case SPELL_DARK_CONTROL_UNDEAD:
                flags |= ON_CAST_TargetCrosshair;
                break;

            case SPELL_EARTH_TELEKINESIS:
                flags |= ON_CAST_Telekenesis;
                break;

            case SPELL_SPIRIT_BLESS:
                if (!skill_value)
                    skill_value = player->pActiveSkills[PLAYER_SKILL_SPIRIT];
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_EXPERT)
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                break;

            case SPELL_SPIRIT_PRESERVATION:
                if (!skill_value)
                    skill_value = player->pActiveSkills[PLAYER_SKILL_SPIRIT];
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_MASTER)
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                break;

            case SPELL_DARK_PAIN_REFLECTION:
                if (!skill_value)
                    skill_value = player->pActiveSkills[PLAYER_SKILL_DARK];
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_MASTER)
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                break;

            case SPELL_BODY_HAMMERHANDS:
                if (!skill_value)
                    skill_value = player->pActiveSkills[PLAYER_SKILL_BODY];
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_GRANDMASTER)
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                break;

            case SPELL_EARTH_STONE_TO_FLESH:
            case SPELL_SPIRIT_REMOVE_CURSE:
            case SPELL_SPIRIT_RAISE_DEAD:
            case SPELL_SPIRIT_RESSURECTION:
            case SPELL_MIND_REMOVE_FEAR:
            case SPELL_MIND_CURE_PARALYSIS:
            case SPELL_MIND_CURE_INSANITY:
            case SPELL_BODY_CURE_WEAKNESS:
            case SPELL_BODY_REGENERATION:
            case SPELL_BODY_CURE_POISON:
            case SPELL_BODY_CURE_DISEASE:
                flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                break;

            case SPELL_DARK_SACRIFICE:
                flags |= ON_CAST_DarkSacrifice;
                break;
            default:
                break;
        }
    }

    // clear previous casts
    if (flags & ON_CAST_CastingInProgress) {
        for (uint i = 0; i < CastSpellInfoCount; ++i)
            if (pCastSpellInfo[i].uFlags & ON_CAST_CastingInProgress) {
                pCastSpellInfo[i].uSpellID = 0;
                break;
            }
    }

    CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress();

    int result = PushCastSpellInfo(spell, uPlayerID, skill_value, flags, a6);
    if (result != -1) {
        Sizei renDims = render->GetRenderDimensions();
        if (flags & ON_CAST_SinglePlayer_BigImprovementAnim) {
            if (pGUIWindow_CastTargetedSpell) return;
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({52, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 0, InputAction::SelectChar1);
            pGUIWindow_CastTargetedSpell->CreateButton({165, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 1, InputAction::SelectChar2);
            pGUIWindow_CastTargetedSpell->CreateButton({280, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 2, InputAction::SelectChar3);
            pGUIWindow_CastTargetedSpell->CreateButton({390, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 3, InputAction::SelectChar4);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_TargetCrosshair) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0,
                UIMSG_CastSpell_Shoot_Monster, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Telekenesis) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0,
                UIMSG_CastSpell_Telekinesis, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Enchantment) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell = pCastSpellInfo[result].GetCastSpellInInventoryWindow();
            _50C9A0_IsEnchantingInProgress = 1;
            some_active_character = uActiveCharacter;
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_MonsterSparkles) {
            if (pGUIWindow_CastTargetedSpell) return;
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({0x34u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 0, InputAction::SelectChar1);
            pGUIWindow_CastTargetedSpell->CreateButton({0xA5u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 1, InputAction::SelectChar2);
            pGUIWindow_CastTargetedSpell->CreateButton({0x118u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 2, InputAction::SelectChar3);
            pGUIWindow_CastTargetedSpell->CreateButton({0x186u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 3, InputAction::SelectChar4);
            pGUIWindow_CastTargetedSpell->CreateButton({8, 8}, {game_viewport_width, game_viewport_height}, 1, 0, UIMSG_CastSpell_Monster_Improvement, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
        }
        if (flags & ON_CAST_DarkSacrifice && !pGUIWindow_CastTargetedSpell) {
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pBtn_NPCLeft = pGUIWindow_CastTargetedSpell->CreateButton({469, 178}, {ui_btn_npc_left->GetWidth(), ui_btn_npc_left->GetHeight()}, 1, 0,
                UIMSG_ScrollNPCPanel, 0, InputAction::Invalid, "", {ui_btn_npc_left});
            pBtn_NPCRight = pGUIWindow_CastTargetedSpell->CreateButton({626, 178}, {ui_btn_npc_right->GetWidth(), ui_btn_npc_right->GetHeight()}, 1, 0,
                UIMSG_ScrollNPCPanel, 1, InputAction::Invalid, "", {ui_btn_npc_right});
            pGUIWindow_CastTargetedSpell->CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_HiredNPC_CastSpell, 4, InputAction::SelectNPC1);
            pGUIWindow_CastTargetedSpell->CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_HiredNPC_CastSpell, 5, InputAction::SelectNPC2);
        }
    }
}
