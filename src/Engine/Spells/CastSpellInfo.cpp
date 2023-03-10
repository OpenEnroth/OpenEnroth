#include <vector>

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

static const size_t CAST_SPELL_QUEUE_SIZE = 10;
static std::array<CastSpellInfo, CAST_SPELL_QUEUE_SIZE> pCastSpellInfo;

/**
 * Common initialization of SpriteObject for spell casting
 *
 * Correct field uType of spritePtr must be set before calling this function
 * because initialization depends on it.
 */
static void initSpellSprite(SpriteObject *spritePtr,
                            PLAYER_SKILL_LEVEL spellLevel,
                            PLAYER_SKILL_MASTERY spellMastery,
                            CastSpellInfo *pCastSpell) {
    assert(spritePtr && spritePtr->uType != SPRITE_NULL);
    assert(pCastSpell->uSpellID != SPELL_NONE);

    spritePtr->containing_item.Reset();
    spritePtr->spell_level = spellLevel;
    spritePtr->uSpellID = pCastSpell->uSpellID;
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
static void spellFailed(CastSpellInfo *pCastSpell,
                        int error_str_id) {
    GameUI_SetStatusBar(error_str_id);
    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
    pCastSpell->uSpellID = SPELL_NONE;
}

void CastSpellInfoHelpers::castSpell() {
    PLAYER_SKILL_TYPE which_skill;
    SpriteObject pSpellSprite;
    AIDirection target_direction;

    bool spell_sound_flag = false;

    static const int ONE_THIRD_PI = TrigLUT.uIntegerPi / 3;

    for (CastSpellInfo &spellInfo : pCastSpellInfo) {  // cycle through spell queue
        CastSpellInfo *pCastSpell = &spellInfo;
        int uRequiredMana, sRecoveryTime;

        if (pCastSpell->uSpellID == SPELL_NONE) {
            continue;  // spell item blank skip to next
        }

        if (pParty->Invisible()) {
            // casting a spell breaks invisibility
            pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (pCastSpell->uFlags & ON_CAST_CastingInProgress) {
            if (!pParty->pPlayers[pCastSpell->uPlayerID].CanAct()) {
                // this cancels the spell cast if the player can no longer act
                cancelSpellCastInProgress();
            }
            continue;
        }

        Player *pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID];

        int spell_targeted_at = pCastSpell->spell_target_pid;

        if (spell_targeted_at == 0) {  // no target ?? test
            bool target_undead;
            if (pCastSpell->uSpellID == SPELL_LIGHT_DESTROY_UNDEAD ||
                pCastSpell->uSpellID == SPELL_SPIRIT_TURN_UNDEAD ||
                pCastSpell->uSpellID == SPELL_DARK_CONTROL_UNDEAD) {
                target_undead = true;
            } else {
                target_undead = false;
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

        pSpellSprite.uType = SpellSpriteMapping[pCastSpell->uSpellID];

        if (pSpellSprite.uType != SPRITE_NULL) {
            if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                int player_pid = PID(OBJECT_Player, pCastSpell->uPlayerID + 1);
                Actor::GetDirectionInfo(player_pid, spell_targeted_at, &target_direction, 0);  // target direciton
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
            which_skill = getSkillTypeForSpell(pCastSpell->uSpellID);

            spell_level = pPlayer->GetActualSkillLevel(which_skill);
            spell_mastery = pPlayer->GetActualSkillMastery(which_skill);

            if (engine->config->debug.AllMagic.Get()) {
                spell_level = 10;
                spell_mastery = PLAYER_SKILL_MASTERY_GRANDMASTER;
            }
        }

        if (isRegularSpell(pCastSpell->uSpellID)) {
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

        if (isRegularSpell(pCastSpell->uSpellID) && pPlayer->sMana < uRequiredMana) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_SPELLPOINTS);
            pCastSpell->uSpellID = SPELL_NONE;
            continue;
        }

        if (pPlayer->IsCursed() && isRegularSpell(pCastSpell->uSpellID) && grng->Random(100) < 50) {  // player is cursed and have a chance to fail spell casting
            if (!pParty->bTurnBasedModeOn) {
                pPlayer->SetRecoveryTime((int64_t)(debug_non_combat_recovery_mul * flt_debugrecmod3 * SPELL_FAILURE_RECOVERY_TIME_ON_CURSE));
            } else {
                pParty->pTurnBasedPlayerRecoveryTimes[pCastSpell->uPlayerID] = SPELL_FAILURE_RECOVERY_TIME_ON_CURSE;
                pPlayer->SetRecoveryTime(sRecoveryTime);
                pTurnEngine->ApplyPlayerAction();
            }

            spellFailed(pCastSpell, LSTR_SPELL_FAILED);
            pPlayer->SpendMana(uRequiredMana);
            return;
        }

        assert(pCastSpell->uSpellID != SPELL_101 && "Unknown spell effect #101 (prolly flaming bow arrow");

        // First process special "pseudo" spells like bow or blaster shots
        // and spells that open additional menus like town portal or lloyd beacon
        if (pCastSpell->uSpellID == SPELL_BOW_ARROW) {
            int arrows = 1;
            if (spell_mastery >= PLAYER_SKILL_MASTERY_MASTER) {
                arrows = 2;
            }

            sRecoveryTime = pPlayer->GetAttackRecoveryTime(true);
            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            if (pPlayer->WearsItem(ITEM_ARTIFACT_ULLYSES, ITEM_SLOT_BOW)) {
                pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BOLT);
            }
            pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
            pSpellSprite.spell_target_pid = spell_targeted_at;
            pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
            pSpellSprite.uFacing = target_direction.uYawAngle;
            pSpellSprite.containing_item = pPlayer->pInventoryItemList[pPlayer->pEquipment.uBow - 1];
            pSpellSprite.uAttributes |= SPRITE_MISSILE;
            if (pParty->bTurnBasedModeOn) {
                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
            }
            for (int i = 0; i < arrows; ++i) {
                if (i) {
                    pSpellSprite.vPosition.z += 32;
                }
                pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                        pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
            }
        } else if (pCastSpell->uSpellID == SPELL_LASER_PROJECTILE) {
            // TODO(Nik-RE-dev): recovery time for melee used?
            sRecoveryTime = pPlayer->GetAttackRecoveryTime(false);
            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            // TODO(pskelton): was pParty->uPartyHeight / 2
            pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
            pSpellSprite.spell_target_pid = spell_targeted_at;
            pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
            pSpellSprite.uFacing = target_direction.uYawAngle;
            pSpellSprite.containing_item = pPlayer->pInventoryItemList[pPlayer->pEquipment.uMainHand - 1];
            // &pParty->pPlayers[pCastSpell->uPlayerID].spellbook.pDarkSpellbook.bIsSpellAvailable[36
            // *
            // pParty->pPlayers[pCastSpell->uPlayerID].pEquipment.uMainHand +
            // 5], );
            pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
            pSpellSprite.uAttributes |= SPRITE_MISSILE;
            if (pParty->bTurnBasedModeOn) {
                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
            }
            int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
            if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                ++pTurnEngine->pending_actions;
            }
        } else if (pCastSpell->uSpellID == SPELL_WATER_TOWN_PORTAL) {
            if (pPlayer->CanCastSpell(uRequiredMana)) {
                int success_chance_percent = 10 * spell_level;
                if (spell_mastery != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                    if (pParty->GetRedOrYellowAlert() ||
                            grng->Random(100) >= success_chance_percent) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                }
                pEventTimer->Pause();
                TownPortalCasterId = pCastSpell->uPlayerID;
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_OnCastTownPortal, 0, 0);
                spell_sound_flag = true;
            } else {
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = SPELL_NONE;
                continue;
            }
        } else if (pCastSpell->uSpellID == SPELL_WATER_LLOYDS_BEACON) {
            if (pCurrentMapName == "d05.blv") {  // Arena
                spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                continue;
            }
            if (pPlayer->CanCastSpell(uRequiredMana)) {
                pEventTimer->Pause();
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_OnCastLloydsBeacon, 0, 0);
                CurrentLloydPlayerID = pCastSpell->uPlayerID;
                LloydsBeaconSpellDuration = GameTime::FromDays(7 * spell_level).GetSeconds();
                pCastSpell->uFlags |= ON_CAST_NoRecoverySpell;
            } else {
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = SPELL_NONE;
                continue;
            }
        } else {
            if (!pPlayer->CanCastSpell(uRequiredMana)) {
                // Not enough mana for current spell, check the next one
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                pCastSpell->uSpellID = SPELL_NONE;
                continue;
            }

            switch (pCastSpell->uSpellID) {
                case SPELL_FIRE_TORCH_LIGHT:
                {
                    int spell_power;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_power = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_power = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_power = 4;
                            break;
                        default:
                            assert(false);
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_FIRE_FIRE_SPIKE:
                {
                    int num_spikes;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            num_spikes = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            num_spikes = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            num_spikes = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            num_spikes = 9;
                            break;
                        default:
                            assert(false);
                    }
                    int spikes_active = 0;
                    for (const SpriteObject &spriteObject : pSpriteObjects) {
                        if (spriteObject.uType &&
                                spriteObject.uSpellID == SPELL_FIRE_FIRE_SPIKE &&
                                spriteObject.spell_caster_pid == PID(OBJECT_Player, pCastSpell->uPlayerID)) {
                            ++spikes_active;
                        }
                    }
                    if (spikes_active > num_spikes) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = (short)target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, pParty->sRotationY + 10, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_AIR_IMPLOSION:
                {
                    int monster_id = PID_ID(spell_targeted_at);
                    if (!spell_targeted_at) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = 0;
                        pSpellSprite.uFacing = 0;
                        pSpellSprite.vPosition = pActors[monster_id].vPosition + Vec3i(0, 0, pActors[monster_id].uActorHeight / 2);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    }
                    break;
                }

                case SPELL_EARTH_MASS_DISTORTION:
                {
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_MASS_DISTORTION]
                            .Apply(GameTime(pMiscTimer->uTotalGameTimeElapsed + 128), PLAYER_SKILL_MASTERY_NONE, 0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = 0;
                        pSpellSprite.uFacing = 0;
                        pSpellSprite.vPosition = pActors[monster_id].vPosition;
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    }
                    break;
                }

                case SPELL_LIGHT_DESTROY_UNDEAD:
                {
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        break;
                    }
                    // v730 = spell_targeted_at >> 3;
                    // HIDWORD(spellduration) =
                    // (int)&pActors[PID_ID(spell_targeted_at)];
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[PID_ID(spell_targeted_at)].vPosition;
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    int obj_id = pSpellSprite.Create(0, 0, 0, 0);
                    if (!MonsterStats::BelongsToSupertype(pActors[PID_ID(spell_targeted_at)].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    Actor::DamageMonsterFromParty(PID(OBJECT_Item, obj_id), PID_ID(spell_targeted_at), &spell_velocity);
                    break;
                }

                case SPELL_FIRE_FIRE_BOLT:
                case SPELL_FIRE_FIREBALL:
                case SPELL_FIRE_INCINERATE:
                case SPELL_AIR_LIGHTNING_BOLT:
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
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    } else {
                        pSpellSprite.uSectorID = 0;
                    }
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pCastSpell->uSpellID == SPELL_AIR_LIGHTNING_BOLT) {
                        pSpellSprite.uAttributes |= SPRITE_SKIP_A_FRAME;
                    }
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_WATER_ACID_BURST:
                case SPELL_EARTH_BLADES:
                case SPELL_BODY_FLYING_FIST:
                case SPELL_DARK_TOXIC_CLOUD:
                {
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    // TODO(pskelton): was pParty->uPartyHeight / 2
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_LIGHT_SUNRAY:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor ||
                            uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
                            (pParty->uCurrentHour < 5 || pParty->uCurrentHour >= 21)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_LIGHT_PARALYZE:
                {
                    int monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor &&
                            pActors[monster_id].DoesDmgTypeDoDamage(DMGT_LIGHT)) {
                        Actor::AI_Stand(PID_ID(spell_targeted_at), 4, 0x80, 0);
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_PARALYZED]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(3 * spell_level), spell_mastery, 0, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        pActors[monster_id].vVelocity.x = 0;
                        pActors[monster_id].vVelocity.y = 0;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_EARTH_SLOW:
                {
                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            spell_power = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 2;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 4;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 8;
                            break;
                        default:
                            assert(false);
                    }
                    // v721 = 836 * PID_ID(spell_targeted_at);
                    int monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor && pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_SLOWED].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_MIND_CHARM:
                {
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                        // Wrong durations from vanilla fixed
                        GameTime spell_duration;

                        switch (spell_mastery) {
                            case PLAYER_SKILL_MASTERY_EXPERT:
                                spell_duration = GameTime::FromMinutes(5 * spell_level);
                                break;
                            case PLAYER_SKILL_MASTERY_MASTER:
                                spell_duration = GameTime::FromMinutes(10 * spell_level);
                                break;
                            case PLAYER_SKILL_MASTERY_GRANDMASTER:
                                // Time must be infinite until the player leaves the map
                                spell_duration = GameTime::FromYears(1);
                                break;
                            case PLAYER_SKILL_MASTERY_NOVICE: // MM6 have different durations
                            default:
                                assert(false);
                        }

                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].vPosition + Vec3i(0, 0, pActors[monster_id].uActorHeight);
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
                    // TODO: spell_id different?
                    initSpellSprite(&pSpellSprite, spell_level * 300, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uSpellID = SPELL_FIRE_PROTECTION_FROM_FIRE;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_DARK_VAMPIRIC_WEAPON:
                case SPELL_FIRE_FIRE_AURA:
                {
                    ItemGen *item = &pParty->pPlayers[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    item->UpdateTempBonus(pParty->GetPlayingTime());
                    if (item->uItemID == ITEM_BLASTER ||
                            item->uItemID == ITEM_BLASTER_RIFLE ||
                            item->IsBroken() ||
                            pItemTable->IsMaterialNonCommon(item) ||
                            item->special_enchantment != ITEM_ENCHANTMENT_NULL ||
                            item->uEnchantmentType != 0 ||
                            !IsWeapon(item->GetItemEquipType())) {
                        AfterEnchClickEventId = UIMSG_Escape;
                        AfterEnchClickEventSecondParam = 0;
                        AfterEnchClickEventTimeout = Timer::Second; // was 1, increased to make message readable
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
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
                        item->uExpireTime = pParty->GetPlayingTime() + GameTime::FromHours(spell_level);
                        item->uAttributes |= ITEM_TEMP_BONUS;
                    }

                    ItemEnchantmentTimer = Timer::Second * 2;
                    break;
                }

                case SPELL_BODY_REGENERATION:
                {
                    int spell_power;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_power = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_power = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_power = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_power = 10;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_REGENERATION]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_FIRE_PROTECTION_FROM_FIRE:
                case SPELL_AIR_PROTECTION_FROM_AIR:
                case SPELL_WATER_PROTECTION_FROM_WATER:
                case SPELL_EARTH_PROTECTION_FROM_EARTH:
                case SPELL_MIND_PROTECTION_FROM_MIND:
                case SPELL_BODY_PROTECTION_FROM_BODY:
                {
                    int spell_power = std::to_underlying(spell_mastery) * spell_level;
                    PARTY_BUFF_INDEX resist;
                    switch (pCastSpell->uSpellID) {
                        case SPELL_FIRE_PROTECTION_FROM_FIRE:
                            resist = PARTY_BUFF_RESIST_FIRE;
                            break;
                        case SPELL_AIR_PROTECTION_FROM_AIR:
                            resist = PARTY_BUFF_RESIST_AIR;
                            break;
                        case SPELL_WATER_PROTECTION_FROM_WATER:
                            resist = PARTY_BUFF_RESIST_WATER;
                            break;
                        case SPELL_EARTH_PROTECTION_FROM_EARTH:
                            resist = PARTY_BUFF_RESIST_EARTH;
                            break;
                        case SPELL_MIND_PROTECTION_FROM_MIND:
                            resist = PARTY_BUFF_RESIST_MIND;
                            break;
                        case SPELL_BODY_PROTECTION_FROM_BODY:
                            resist = PARTY_BUFF_RESIST_BODY;
                            break;
                        default:
                            assert(false);
                            continue;
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);

                    pParty->pPartyBuffs[resist].Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_FIRE_HASTE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(4 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    bool has_weak = false;
                    for (const Player &player : pParty->pPlayers) {
                        if (player.conditions.Has(Condition_Weak)) {
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
                    pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    break;
                }

                case SPELL_SPIRIT_BLESS:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level + 1);
                            break;
                        default:
                            assert(false);
                    }

                    int spell_power = spell_level + 5;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        int spell_overlay_id = pOtherOverlayList->_4418B1(10000, pCastSpell->uPlayerID_2 + 310, 0, 65536);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_BLESS]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, spell_overlay_id, 0);
                    } else {
                        for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                            int spell_overlay_id = pOtherOverlayList->_4418B1(10000, i + 310, 0, 65536);
                            pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_BLESS]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, spell_overlay_id, 0);
                        }
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    }
                    break;
                }

                case SPELL_SPIRIT_SPIRIT_LASH:
                {
                    if (spell_targeted_at && PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        int monster_id = PID_ID(spell_targeted_at);
                        float dist = (pActors[monster_id].vPosition - pParty->vPosition).ToFloat().Length();
                        if (dist <= 307.2) {
                            Vec3i spell_velocity = Vec3i(0, 0, 0);
                            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                            pSpellSprite.uSectorID = 0;
                            pSpellSprite.field_60_distance_related_prolly_lod = 0;
                            pSpellSprite.uFacing = 0;
                            pSpellSprite.vPosition = pActors[monster_id].vPosition - Vec3i(0, 0, pActors[monster_id].uActorHeight * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, spell_targeted_at);
                            Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                        } else {
                            spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                            pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                            continue;
                        }
                    } else {
                        // Previous behavoiur - decrease mana but do not play sound if
                        // spell is targeted wrong
                        // TODO: revisit, maybe we want to properly play sound and display a message here?
                        pPlayer->SpendMana(uRequiredMana);
                        pCastSpell->uSpellID = SPELL_NONE;
                        continue;
                    }
                    break;
                }

                case SPELL_AIR_SHIELD:
                case SPELL_EARTH_STONESKIN:
                case SPELL_SPIRIT_HEROISM:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level + 1);
                            break;
                        default:
                            assert(false);
                    }

                    int spell_power;
                    PARTY_BUFF_INDEX buff;
                    switch (pCastSpell->uSpellID) {
                        case SPELL_AIR_SHIELD:
                            spell_power = 0;
                            buff = PARTY_BUFF_SHIELD;
                            break;
                        case SPELL_EARTH_STONESKIN:
                            spell_power = spell_level + 5;
                            buff = PARTY_BUFF_STONE_SKIN;
                            break;
                        case SPELL_SPIRIT_HEROISM:
                            spell_power = spell_level + 5;
                            buff = PARTY_BUFF_HEROISM;
                            break;
                        default:
                            assert(false);
                            continue;
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[buff]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_FIRE_IMMOLATION:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_level, 0, 0);
                    break;
                }

                case SPELL_FIRE_METEOR_SHOWER:
                {
                    if (spell_mastery < PLAYER_SKILL_MASTERY_MASTER) {
                        pCastSpell->uSpellID = SPELL_NONE;
                        continue;
                    }

                    int meteor_num;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        meteor_num = 20;
                    } else {
                        meteor_num = 16;
                    }
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        spellFailed(pCastSpell, LSTR_CANT_METEOR_SHOWER_INDOORS);
                        continue;
                    }
                    ObjectType obj_type = PID_TYPE(spell_targeted_at);
                    int monster_id = PID_ID(spell_targeted_at);
                    Vec3i dist;
                    if (obj_type == OBJECT_Actor) {  // quick cast can specify target
                        dist = pActors[monster_id].vPosition;
                    } else {
                        dist = pParty->vPosition + Vec3i(2048 * pCamera3D->fRotationZCosine, 2048 * pCamera3D->fRotationZSine, 0);
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
                            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                            pSpellSprite.vPosition = dist + Vec3i(0, 0, spell_targeted_at);
                            pSpellSprite.uSectorID = 0;
                            pSpellSprite.spell_target_pid = target_pid;
                            pSpellSprite.field_60_distance_related_prolly_lod = stru_50C198._427546(spell_targeted_at + 2500);
                            pSpellSprite.uFacing = yaw;
                            if (pParty->bTurnBasedModeOn) {
                                pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                            }
                            int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                            if (pSpellSprite.Create(yaw, pitch, spell_speed, 0) != -1 &&
                                    pParty->bTurnBasedModeOn) {
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
                        spellFailed(pCastSpell, LSTR_CANT_INFERNO_OUTDOORS);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (Actor* actor : render->getActorsInViewport(4096)) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), actor->id, &spell_velocity);
                        spell_fx_renderer->RenderAsSprite(&pSpellSprite);
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.OrangeyRed.C32(), 0x40);
                    break;
                }

                case SPELL_AIR_WIZARD_EYE:
                {
                    // TODO(Nik-RE-dev): it picks overlay for last character?
                    int spell_overlay_id;
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        spell_overlay_id = pOtherOverlayList->_4418B1(2000, i + 100, 0, 65536);
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, 0, spell_overlay_id, 0);
                    break;
                }

                case SPELL_AIR_FEATHER_FALL:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        default:
                            assert(false);
                    }
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        pOtherOverlayList->_4418B1(2010, i + 100, 0, 65536);
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);

                    pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_AIR_SPARKS:
                {
                    int sparks_number;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            sparks_number = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            sparks_number = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            sparks_number = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            sparks_number = 9;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_spray_angle_start = ONE_THIRD_PI / -2;
                    int spell_spray_angle_end = ONE_THIRD_PI / 2;
                    while (spell_spray_angle_start <= spell_spray_angle_end) {
                        pSpellSprite.uSpriteFrameID = grng->Random(64); // TODO(captainurist): this doesn't look like a frame id initialization
                        pSpellSprite.uFacing = spell_spray_angle_start + (short)target_direction.uYawAngle;
                        int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                        int yaw = spell_spray_angle_start + target_direction.uYawAngle;
                        if (pSpellSprite.Create(yaw, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        spell_spray_angle_start += ONE_THIRD_PI / (sparks_number - 1);
                    }
                    break;
                }

                case SPELL_AIR_JUMP:
                {
                    if (pParty->IsAirborne()) {
                        spellFailed(pCastSpell, LSTR_CANT_JUMP_AIRBORNE);
                        continue;
                    }
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        pOtherOverlayList->_4418B1(2040, i + 100, 0, 65536);
                    }
                    pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                    pParty->uFallSpeed = 1000;
                    break;
                }

                case SPELL_AIR_INVISIBILITY:
                {
                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            spell_power = 3 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            spell_power = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    if (pParty->GetRedOrYellowAlert()) {
                        spellFailed(pCastSpell, LSTR_HOSTILE_CREATURES_NEARBY);
                        continue;
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_AIR_FLY:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        spellFailed(pCastSpell, LSTR_CANT_FLY_INDOORS);
                        continue;
                    }
                    if (!pParty->pPlayers[pCastSpell->uPlayerID].GetMaxMana() && !engine->config->debug.AllMagic.Get()) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        pOtherOverlayList->_4418B1(2090, i + 100, 0, 65536);
                    }
                    int spell_overlay_id = pOtherOverlayList->_4418B1(10008, 203, 0, 65536);
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].Apply(
                            pParty->GetPlayingTime() + GameTime::FromHours(spell_level),
                            spell_mastery, 0, spell_overlay_id,
                            pCastSpell->uPlayerID + 1);
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff = (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER);
                    break;
                }

                case SPELL_AIR_STARBURST:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        spellFailed(pCastSpell, LSTR_CANT_STARBURST_INDOORS);
                        continue;
                    }
                    ObjectType obj_type = PID_TYPE(spell_targeted_at);
                    int monster_id = PID_ID(spell_targeted_at);
                    Vec3i dist;
                    if (obj_type == OBJECT_Actor) {  // quick cast can specify target
                        dist = pActors[monster_id].vPosition;
                    } else {
                        dist = pParty->vPosition + Vec3i(2048 * pCamera3D->fRotationZCosine, 2048 * pCamera3D->fRotationZSine, 0);
                    }
                    uint64_t k = 0;
                    int j = 0;
                    int target_pid = obj_type == OBJECT_Actor ? spell_targeted_at : 0;
                    uint32_t yaw, pitch;
                    for (uint star_num = 20; star_num; star_num--) {
                        spell_targeted_at = grng->Random(1000);
                        if (sqrt(((double)spell_targeted_at + (double)dist.z -
                                        (double)(dist.z + 2500)) *
                                    ((double)spell_targeted_at + (double)dist.z -
                                     (double)(dist.z + 2500)) +
                                    j * j + k * k) <= 1.0) {
                            yaw = 0;
                            pitch = 0;
                        } else {
                            pitch = TrigLUT.Atan2((int64_t)sqrt((float)(j * j + k * k)),
                                    ((double)spell_targeted_at + (double)dist.z -
                                     (double)(dist.z + 2500)));
                            yaw = TrigLUT.Atan2(j, k);
                        }
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = dist + Vec3i(0, 0, spell_targeted_at + 2500);
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.spell_target_pid = target_pid;
                        pSpellSprite.field_60_distance_related_prolly_lod = stru_50C198._427546(spell_targeted_at + 2500);
                        pSpellSprite.uFacing = yaw;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                        if (pSpellSprite.Create(yaw, pitch, spell_speed, 0) != -1 &&
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
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    for (Player &player : pParty->pPlayers) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            if (player.conditions.Has(Condition_Sleep)) {
                                player.conditions.Reset(Condition_Sleep);
                                player.PlaySound(SPEECH_Awaken, 0);
                            }
                        } else {
                            if (player.DiscardConditionIfLastsLongerThan(Condition_Sleep, pParty->GetPlayingTime() - spell_duration)) {
                                player.PlaySound(SPEECH_Awaken, 0);
                            }
                        }
                    }
                    break;
                }

                case SPELL_WATER_POISON_SPRAY:
                {
                    int shots_num;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            shots_num = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            shots_num = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            shots_num = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            shots_num = 7;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (shots_num == 1) {
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        if (pSpellSprite.Create(target_direction.uYawAngle, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                    } else {
                        int spell_spray_angle_start = ONE_THIRD_PI / -2;
                        int spell_spray_angle_end = ONE_THIRD_PI / 2;
                        if (spell_spray_angle_start <= spell_spray_angle_end) {
                            do {
                                pSpellSprite.uFacing = spell_spray_angle_start + target_direction.uYawAngle;
                                if (pSpellSprite.Create(pSpellSprite.uFacing, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                                        pParty->bTurnBasedModeOn) {
                                    ++pTurnEngine->pending_actions;
                                }
                                spell_spray_angle_start += ONE_THIRD_PI / (shots_num - 1);
                            } while (spell_spray_angle_start <= spell_spray_angle_end);
                        }
                    }
                    break;
                }

                case SPELL_WATER_WATER_WALK:
                {
                    if (!pParty->pPlayers[pCastSpell->uPlayerID].GetMaxMana()) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }

                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    int spell_overlay_id = pOtherOverlayList->_4418B1(10005, 201, 0, 65536);
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, spell_overlay_id, pCastSpell->uPlayerID + 1);
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff = (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER);
                    break;
                }

                case SPELL_WATER_RECHARGE_ITEM:
                {
                    ItemGen *item = &pParty->pPlayers[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    if (item->GetItemEquipType() != EQUIP_WAND || item->IsBroken()) {
                        AfterEnchClickEventId = UIMSG_Escape;
                        AfterEnchClickEventSecondParam = 0;
                        AfterEnchClickEventTimeout = Timer::Second; // was 1, increased to make message readable
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
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
                        AfterEnchClickEventId = UIMSG_Escape;
                        AfterEnchClickEventSecondParam = 0;
                        AfterEnchClickEventTimeout = Timer::Second; // was 1, increased to make message readable
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }

                    item->uAttributes |= ITEM_AURA_EFFECT_GREEN;
                    ItemEnchantmentTimer = Timer::Second * 2;
                    break;
                }

                case SPELL_WATER_ENCHANT_ITEM:
                {
                    uRequiredMana = 0;
                    int success_chance_percent = 10 * spell_level; // 10% chance of success per spell level
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
                            if (rnd >= success_chance_percent) {
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
                                    ItemEnchantmentTimer = Timer::Second * 2;
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
                                    ItemEnchantmentTimer = Timer::Second * 2;
                                    spell_failed = false;
                                }
                            }
                        }
                    }

                    if (spell_failed) {
                        spellFailed(pCastSpell, item_not_broken ? LSTR_SPELL_FAILED : LSTR_ITEM_TOO_LAME);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_SpellFailed, 0);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    break;
                }

                case SPELL_EARTH_STONE_TO_FLESH:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Petrified)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Petrified);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Petrified, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_EARTH_ROCK_BLAST:
                {
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = pParty->sRotationZ;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, pParty->sRotationY, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_EARTH_DEATH_BLOSSOM:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    pSpellSprite.uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.uFacing = (short)pParty->sRotationZ;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->sRotationZ, TrigLUT.uIntegerHalfPi / 2, spell_speed, 0) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_SPIRIT_DETECT_LIFE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(30 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_SPIRIT_FATE:
                {
                    int spell_power;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_power = spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_power = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_power = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_power = 6 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    // LODWORD(spellduration) = 300;
                    if (pCastSpell->spell_target_pid == 0) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_FATE]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(5), spell_mastery, spell_power, 0, 0);
                    } else if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        int monster_id = PID_ID(pCastSpell->spell_target_pid);
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_FATE]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(5), spell_mastery, spell_power, 0, 0);
                        pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], 0);
                    }
                    break;
                }

                case SPELL_SPIRIT_REMOVE_CURSE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Cursed)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Cursed);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Cursed, pParty->GetPlayingTime() - spell_duration);
                        }
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Cursed)) {
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_PRESERVATION:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }

                    if (spell_mastery == PLAYER_SKILL_MASTERY_NOVICE || spell_mastery == PLAYER_SKILL_MASTERY_EXPERT) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    } else {
                        for (Player &player : pParty->pPlayers) {
                            player.pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        }
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    }
                    break;
                }

                case SPELL_SPIRIT_TURN_UNDEAD:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 + 1 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            // Was "3m + 1m * spell_level"
                            spell_duration = GameTime::FromMinutes(3 + 3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(3 + 5 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_SPIRIT_TURN_UNDEAD_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (Actor* actor : render->getActorsInViewport(4096)) {
                        if (MonsterStats::BelongsToSupertype(actor->pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                            pSpellSprite.Create(0, 0, 0, 0);
                            actor->pActorBuffs[ACTOR_BUFF_AFRAID].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.White.C32(), 192);
                    break;
                }

                case SPELL_SPIRIT_RAISE_DEAD:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT: // MM6 only
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    pOtherOverlayList->_4418B1(5080, pCastSpell->uPlayerID_2 + 100, 0, 65536);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Dead)) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Dead);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Unconscious);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Dead, pParty->GetPlayingTime() - spell_duration);
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Unconscious, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(Condition_Weak, 0);
                    }
                    break;
                }

                case SPELL_SPIRIT_SHARED_LIFE:
                {
                    int shared_life_count;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        shared_life_count = 4 * spell_level;
                    } else {
                        shared_life_count = 3 * spell_level;
                    }
                    int active_pl_num = 0;
                    for (const Player &player : pParty->pPlayers) {
                        if (player.conditions.HasNone({Condition_Dead, Condition_Petrified, Condition_Eradicated})) {
                            shared_life_count += player.sHealth;
                            active_pl_num++;
                        }
                    }
                    int mean_life = shared_life_count / active_pl_num;
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        if (pParty->pPlayers[i].conditions.HasNone({Condition_Dead, Condition_Petrified, Condition_Eradicated})) {
                            pParty->pPlayers[i].sHealth = mean_life;
                            if (pParty->pPlayers[i].sHealth > pParty->pPlayers[i].GetMaxHealth()) {
                                pParty->pPlayers[i].sHealth = pParty->pPlayers[i].GetMaxHealth();
                            }
                            if (pParty->pPlayers[i].sHealth > 0) {
                                pParty->pPlayers[i].SetUnconcious(GameTime(0));
                            }
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, i);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_RESSURECTION:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT: // MM6 only
                            // Was 3 hours per spell level
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER: // MM6 only
                            // Was 3 days per spell level
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.HasAny({Condition_Eradicated, Condition_Dead})) {
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak)) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_Weak, 0);
                        }
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Eradicated);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Dead);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Unconscious);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Eradicated, pParty->GetPlayingTime() - spell_duration);
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Dead, pParty->GetPlayingTime() - spell_duration);
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Unconscious, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(Condition_Weak, 1);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    break;
                }

                case SPELL_MIND_CURE_PARALYSIS:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
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
                                .DiscardConditionIfLastsLongerThan(Condition_Paralyzed, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_MIND_REMOVE_FEAR:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
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
                                .DiscardConditionIfLastsLongerThan(Condition_Fear, pParty->GetPlayingTime() - spell_duration);
                        }
                    }

                    break;
                }

                case SPELL_MIND_TELEPATHY:
                {
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        int monster_id = PID_ID(spell_targeted_at);
                        if (!pActors[monster_id].ActorHasItem()) {
                            pActors[monster_id].SetRandomGoldIfTheresNoItem();
                        }
                        int gold_num = 0;
                        if (pActors[monster_id].ActorHasItems[3].uItemID != ITEM_NULL) {
                            if (pItemTable->pItems[pActors[monster_id].ActorHasItems[3].uItemID].uEquipType == EQUIP_GOLD) {
                                gold_num = pActors[monster_id].ActorHasItems[3].special_enchantment;
                            }
                        }
                        ItemGen item;
                        item.Reset();
                        if (pActors[monster_id].uCarriedItemID != ITEM_NULL) {
                            item.uItemID = pActors[monster_id].uCarriedItemID;
                        } else {
                            for (const ItemGen &actorItem : pActors[monster_id].ActorHasItems) {
                                if (actorItem.uItemID != ITEM_NULL &&
                                        pItemTable->pItems[actorItem.uItemID].uEquipType != EQUIP_GOLD) {
                                    item = actorItem;
                                }
                            }
                        }
                        if (gold_num > 0) {
                            if (item.uItemID != ITEM_NULL)
                                GameUI_SetStatusBar(fmt::format("({}), and {} gold", item.GetDisplayName(), gold_num));
                            else
                                GameUI_SetStatusBar(fmt::format("{} gold", gold_num));
                        } else {
                            if (item.uItemID != ITEM_NULL) {
                                GameUI_SetStatusBar(fmt::format("({})", item.GetDisplayName()));
                            } else {
                                GameUI_SetStatusBar("nothing");
                            }
                        }

                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        // TODO(Nik-RE-dev): using just monster height for Z coord looks like a bug
                        pSpellSprite.vPosition = Vec3i(pActors[monster_id].vPosition.x, pActors[monster_id].vPosition.y, pActors[monster_id].uActorHeight);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_BERSERK:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            // TODO: is this correct?
                            // Spell description says that spell effect is infinite.
                            spell_duration = GameTime::FromHours(1);
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        // v730 = 836 * monster_id;
                        if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Reset();
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                            pActors[monster_id].pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Long;
                        }
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].vPosition + Vec3i(0, 0, pActors[monster_id].uActorHeight);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_ENSLAVE:
                {
                    GameTime spell_duration = GameTime::FromMinutes(10 * spell_level);

                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        int monster_id = PID_ID(spell_targeted_at);
                        // v730 = 836 * monster_id;
                        if (MonsterStats::BelongsToSupertype(pActors[monster_id].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            break;
                        }
                        if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                            pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        }
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].vPosition + Vec3i(0, 0, pActors[monster_id].uActorHeight);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_MIND_MASS_FEAR:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                        case PLAYER_SKILL_MASTERY_EXPERT: // MM6 only
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_MIND_MASS_FEAR_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    for (Actor* actor : render->getActorsInViewport(4096)) {
                        // Change: do not exit loop when first undead monster is found
                        if (!MonsterStats::BelongsToSupertype(actor->pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                            pSpellSprite.Create(0, 0, 0, 0);
                            if (actor->DoesDmgTypeDoDamage(DMGT_MIND)) {
                                actor->pActorBuffs[ACTOR_BUFF_AFRAID].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                            }
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Night.C32(), 192);
                    break;
                }

                case SPELL_MIND_CURE_INSANITY:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT: // MM6 only
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Insane)) {
                        if (!pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak)) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_Weak, 0);
                        }
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Insane);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Insane, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition( Condition_Weak, 0);
                    }
                    break;
                }

                case SPELL_EARTH_TELEKINESIS:
                {
                    int obj_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Item) {
                        if (pItemTable->pItems[pSpriteObjects[obj_id].containing_item.uItemID].uEquipType == EQUIP_GOLD) {
                            pParty->PartyFindsGold(pSpriteObjects[obj_id].containing_item.special_enchantment, 0);
                        } else {
                            GameUI_SetStatusBar(LSTR_FMT_YOU_FOUND_ITEM,
                                    pItemTable->pItems[pSpriteObjects[obj_id].containing_item.uItemID].pUnidentifiedName);
                            if (!pParty->AddItemToParty(&pSpriteObjects[obj_id].containing_item)) {
                                pParty->SetHoldingItem(&pSpriteObjects[obj_id].containing_item);
                            }
                        }
                        SpriteObject::OnInteraction(obj_id);
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        pActors[obj_id].LootActor();
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Decoration) {
                        OpenedTelekinesis = true;
                        if (pLevelDecorations[obj_id].uEventID) {
                            EventProcessor(pLevelDecorations[obj_id].uEventID, spell_targeted_at, 1);
                        }
                        // TODO(captainurist): investigate, that's a very weird std::to_underlying call.
                        if (pLevelDecorations[std::to_underlying(pSpriteObjects[obj_id].containing_item.uItemID)].IsInteractive()) {
                            activeLevelDecoration = &pLevelDecorations[obj_id];
                            EventProcessor(stru_5E4C90_MapPersistVars._decor_events[pLevelDecorations[obj_id]._idx_in_stru123 - 75] + 380, 0, 1);
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
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Weak)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Weak);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Weak, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_BODY_FIRST_AID:
                {
                    int heal_amount;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            heal_amount = 2 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            heal_amount = 3 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            heal_amount = 4 * spell_level + 5;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            heal_amount = 5 * spell_level + 5;
                            break;
                        default:
                            assert(false);
                    }
                    if (!pCastSpell->spell_target_pid) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2].Heal(heal_amount);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        int monster_id = PID_ID(pCastSpell->spell_target_pid);
                        if (pActors[monster_id].uAIState != Dead &&
                                pActors[monster_id].uAIState != Dying &&
                                pActors[monster_id].uAIState != Disabled &&
                                pActors[monster_id].uAIState != Removed) {
                            pActors[monster_id].sCurrentHP += heal_amount;
                            if (pActors[monster_id].sCurrentHP > pActors[monster_id].pMonsterInfo.uHP) {
                                pActors[monster_id].sCurrentHP = pActors[monster_id].pMonsterInfo.uHP;
                            }
                        }
                    }
                    break;
                }

                case SPELL_BODY_CURE_POISON:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Weak) ||
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Medium) ||
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Poison_Severe)) {
                        if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Weak);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Medium);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Reset(Condition_Poison_Severe);
                        } else {
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Poison_Weak, pParty->GetPlayingTime() - spell_duration);
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Poison_Medium, pParty->GetPlayingTime() - spell_duration);
                            pParty->pPlayers[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(Condition_Poison_Severe, pParty->GetPlayingTime() - spell_duration);
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
                    pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, 0, 0);
                    break;
                }

                case SPELL_BODY_HAMMERHANDS:
                {
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                        for (Player &player : pParty->pPlayers) {
                            player.pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                                .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, spell_level, 0);
                        }
                    } else {
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, spell_level, 0);
                    }
                    break;
                }

                case SPELL_BODY_POWER_CURE:
                {
                    for (Player &player : pParty->pPlayers) {
                        player.Heal(5 * spell_level + 10);
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    break;
                }

                case SPELL_LIGHT_DISPEL_MAGIC:
                {
                    sRecoveryTime -= spell_level;
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_DISPEL_MAGIC_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    // Spell damage processing was removed because Dispel Magic does not do damage
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        pSpellSprite.Create(0, 0, 0, 0);
                        for (SpellBuff &buff : actor->pActorBuffs) {
                            buff.Reset();
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.VibrantGreen.C32(), 192);
                    break;
                }

                case SPELL_LIGHT_SUMMON_ELEMENTAL:
                {
                    GameTime spell_duration;
                    int max_summoned;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            max_summoned = 1;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(15 * spell_level);
                            max_summoned = 3;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(15 * spell_level);
                            max_summoned = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int mon_num = 0;
                    for (const Actor &actor : pActors) {
                        if (actor.uAIState != Dead &&
                                actor.uAIState != Removed &&
                                actor.uAIState != Disabled &&
                                PID(OBJECT_Player, pCastSpell->uPlayerID) == actor.uSummonerID) {
                            ++mon_num;
                        }
                    }
                    if (mon_num >= max_summoned) {
                        spellFailed(pCastSpell, LSTR_SUMMONS_LIMIT_REACHED);
                        continue;
                    }
                    Spawn_Light_Elemental(pCastSpell->uPlayerID, spell_mastery, spell_duration.GetSeconds());
                    break;
                }

                case SPELL_LIGHT_DAY_OF_THE_GODS:
                {
                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(3 * spell_level);
                            spell_power = 3 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(4 * spell_level);
                            spell_power = 4 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(5 * spell_level);
                            spell_power = 5 * spell_level + 10;
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE: // MM6 have different durations
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_LIGHT_PRISMATIC_LIGHT:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                        spellFailed(pCastSpell, LSTR_CANT_PRISMATIC_OUTDOORS);
                        continue;
                    }
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), actor->id, &spell_velocity);
                    }
                    spell_fx_renderer->_4A8BFC_prismatic_light();
                    break;
                }

                case SPELL_LIGHT_DAY_OF_PROTECTION:
                {
                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(4 * spell_level);
                            spell_power = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(5 * spell_level);
                            spell_power = 5 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE: // In MM6 this spell is different and is of dark magic
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_BODY].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_MIND].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_FIRE].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_WATER].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_AIR].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_RESIST_EARTH].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    // Spell power for Feather fall and Wizard eye was "spell_level + 5"
                    // Changed it to 0 because spell power isn't used for these spells.
                    pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_LIGHT_HOUR_OF_POWER:
                {
                    // Spell power for subsequent buffs is the same as for their single spell versions
                    // Which somehow contradicts with spell description in-game about power being
                    // 4 times light skill for Master and 5 times light skill for Grandmaster
                    PLAYER_SKILL_LEVEL target_skill_level = spell_level + 5;
                    GameTime haste_duration, other_duration;
                    int target_spell_level;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_MASTER:
                            // Was "1h * spell_level + 1m" for other buffs
                            target_spell_level = spell_level * 4;
                            haste_duration = GameTime::FromHours(1).AddMinutes(3 * target_spell_level);
                            other_duration = GameTime::FromHours(1).AddMinutes(15 * target_spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            // In-game description says that the duration for non-haste buffs is "1h + (1h * spell_level) * 5",
                            // but what we have here is "1h + (15m * spell_level) * 5".
                            // And in the original decompiled code it was "1m + (15m * spell_level) * 5".
                            // Note that durations that are used for non-haste buffs here are actually durations that are used
                            // when casting individual spells on master, even when the hour of power itself is cast on GM. E.g.
                            // stone skin on GM lasts "1h + 1h * earth_spell_level", but when cast as part of a GM hour of power
                            // it only lasts "1h + (1h + 15m) * light_spell_level".
                            target_spell_level = spell_level * 5;
                            haste_duration = GameTime::FromHours(1).AddMinutes(4 * target_spell_level);
                            other_duration = GameTime::FromHours(1).AddMinutes(15 * target_spell_level);
                            break;
                        // Novice and master durations was
                        // "1h + 4m * spell_level" for haste
                        // "1m + 20min * spell_level" for other buffs
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    bool player_weak = false;
                    for (Player &player : pParty->pPlayers) {
                       player.pPlayerBuffs[PLAYER_BUFF_BLESS]
                            .Apply(pParty->GetPlayingTime() + other_duration, spell_mastery, target_skill_level, 0, 0);
                        if (player.conditions.Has(Condition_Weak)) {
                            player_weak = true;
                        }
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);

                    pParty->pPartyBuffs[PARTY_BUFF_HEROISM]
                        .Apply(pParty->GetPlayingTime() + other_duration, spell_mastery, target_skill_level, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_SHIELD]
                        .Apply(pParty->GetPlayingTime() + other_duration, spell_mastery, 0, 0, 0);
                    pParty->pPartyBuffs[PARTY_BUFF_STONE_SKIN]
                        .Apply(pParty->GetPlayingTime() + other_duration, spell_mastery, target_skill_level, 0, 0);
                    if (!player_weak) {
                        // Spell power was changed to 0 because it is not used in Haste buff
                        pParty->pPartyBuffs[PARTY_BUFF_HASTE]
                            .Apply(pParty->GetPlayingTime() + haste_duration, spell_mastery, 0, 0, 0);
                    }
                    break;
                }

                case SPELL_LIGHT_DIVINE_INTERVENTION:
                {
                    if (pPlayer->uNumDivineInterventionCastsThisDay >= 3) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    for (Player &player : pParty->pPlayers) {
                        player.conditions.ResetAll();
                        player.sHealth = player.GetMaxHealth();
                        player.sMana = player.GetMaxMana();
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
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
                    int target_monster_level;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            target_monster_level = 2 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            target_monster_level = 3 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            target_monster_level = 4 * spell_level;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            target_monster_level = 5 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    int zombie_hp_limit = target_monster_level * 10;
                    if (!pCastSpell->spell_target_pid) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        if (pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Has(Condition_Dead)) {
                            pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(Condition_Zombie, 1);
                            GameUI_ReloadPlayerPortraits(pCastSpell->uPlayerID_2, (pParty->pPlayers[pCastSpell->uPlayerID_2].GetSexByVoice() != 0) + 23);
                            pParty->pPlayers[pCastSpell->uPlayerID_2].conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
                            // TODO: why call SetCondition and then conditions.Set?
                        }
                        break;
                    }
                    int monster_id = PID_ID(pCastSpell->spell_target_pid);
                    if (monster_id == -1) {
                        spellFailed(pCastSpell, LSTR_NO_VALID_SPELL_TARGET);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    if (pActors[monster_id].sCurrentHP > 0 || pActors[monster_id].uAIState != Dead && pActors[monster_id].uAIState != Dying) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_DARK_REANIMATE_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    pSpellSprite.vPosition = pActors[monster_id].vPosition - Vec3i(0, 0, pActors[monster_id].uActorHeight * -0.8);
                    pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                    pSpellSprite.Create(0, 0, 0, 0);
                    if (pActors[monster_id].pMonsterInfo.uLevel > target_monster_level) {
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
                    if (pActors[monster_id].sCurrentHP > zombie_hp_limit) {
                        pActors[monster_id].sCurrentHP = zombie_hp_limit;
                    }
                    break;
                }

                case SPELL_DARK_SHARPMETAL:
                {
                    int blades_cound;
                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_NOVICE:
                            blades_cound = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            blades_cound = 5;
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            blades_cound = 7;
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            blades_cound = 9;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    // TODO(pskelton): was pParty->uPartyHeight / 2
                    pSpellSprite.vPosition = pParty->vPosition + Vec3i(0, 0, pParty->uPartyHeight / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_spray_angle_start = ONE_THIRD_PI / -2;
                    int spell_spray_angle_end = ONE_THIRD_PI / 2;
                    if (spell_spray_angle_start <= spell_spray_angle_end) {
                        do {
                            pSpellSprite.uFacing = spell_spray_angle_start + target_direction.uYawAngle;
                            int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                            if (pSpellSprite.Create(pSpellSprite.uFacing, target_direction.uPitchAngle, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                                    pParty->bTurnBasedModeOn) {
                                ++pTurnEngine->pending_actions;
                            }
                            spell_spray_angle_start += ONE_THIRD_PI / (blades_cound - 1);
                        } while (spell_spray_angle_start <= spell_spray_angle_end);
                    }
                    break;
                }

                case SPELL_DARK_CONTROL_UNDEAD:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            // Time must be infinite until the player leaves the map
                            spell_duration = GameTime::FromYears(1);
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                        int monster_id = PID_ID(spell_targeted_at);
                        if (!MonsterStats::BelongsToSupertype(pActors[monster_id].pMonsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            break;
                        }
                        if (!pActors[monster_id].DoesDmgTypeDoDamage(DMGT_DARK)) {
                            spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                            pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                            continue;
                        }
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                        pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].vPosition + Vec3i(0, 0, pActors[monster_id].uActorHeight);
                        pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                        pSpellSprite.spell_target_pid = spell_targeted_at;
                        pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                        pSpellSprite.uFacing = target_direction.uYawAngle;
                        pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                        pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    }
                    break;
                }

                case SPELL_DARK_SACRIFICE:
                {
                    memset(&achieved_awards, 0, 4000);
                    for (int npc_id = 0, hired_npc = 0; NPCData &HiredNPC : pParty->pHirelings) {
                        if (!HiredNPC.pName.empty()) {
                            achieved_awards[hired_npc++] = (AwardType)(npc_id + 1);
                        }
                        npc_id++;
                    }

                    if (pCastSpell->uPlayerID_2 != 4 && pCastSpell->uPlayerID_2 != 5 ||
                            achieved_awards[pCastSpell->uPlayerID_2 - 4] <= 0 ||
                            achieved_awards[pCastSpell->uPlayerID_2 - 4] >= 3) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        continue;
                    }
                    int hireling_idx = achieved_awards[pCastSpell->uPlayerID_2 - 4] - 1;
                    pParty->pHirelings[hireling_idx].dialogue_1_evt_id = 1;
                    pParty->pHirelings[hireling_idx].dialogue_2_evt_id = 0;
                    pParty->pHirelings[hireling_idx].dialogue_3_evt_id = pIconsFrameTable->GetIcon("spell96")->GetAnimLength();
                    for (Player &player : pParty->pPlayers) {
                        player.sHealth = player.GetMaxHealth();
                        player.sMana = player.GetMaxMana();
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
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case PLAYER_SKILL_MASTERY_EXPERT:
                        case PLAYER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                        case PLAYER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case PLAYER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int spell_power = spell_level + 5;
                    if (spell_mastery != PLAYER_SKILL_MASTERY_MASTER && spell_mastery != PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pPlayers[pCastSpell->uPlayerID_2].pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    } else {
                        for (Player &player : pParty->pPlayers) {
                            player.pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                        }
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    }
                    break;
                }

                case SPELL_DARK_SOULDRINKER:
                {
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    std::vector<Actor*> actorsInViewport = render->getActorsInViewport(pCamera3D->GetMouseInfoDepth());
                    for (Actor *actor : actorsInViewport) {
                        pSpellSprite.vPosition = actor->vPosition - Vec3i(0, 0, actor->uActorHeight * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), actor->id, &spell_velocity);
                    }
                    int drained_health = (actorsInViewport.size() * (7 * spell_level + 25));
                    int active_pl_num = 0;
                    for (const Player &player : pParty->pPlayers) {
                        if (player.CanAct()) {
                            active_pl_num++;
                        }
                    }
                    for (size_t i = 0; i < pParty->pPlayers.size(); i++) {
                        if (pParty->pPlayers[i].CanAct()) {
                            pParty->pPlayers[i].sHealth += drained_health / active_pl_num;
                            if (pParty->pPlayers[i].sHealth > pParty->pPlayers[i].GetMaxHealth()) {
                                pParty->pPlayers[i].sHealth = pParty->pPlayers[i].GetMaxHealth();
                            }
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, i);
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Black.C32(), 64);
                    break;
                }

                case SPELL_DARK_ARMAGEDDON:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                        spellFailed(pCastSpell, LSTR_CANT_ARMAGEDDON_INDOORS);
                        continue;
                    }
                    int max_casts_a_day;
                    if (spell_mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {
                        max_casts_a_day = 4;
                    } else {
                        max_casts_a_day = 3;
                    }
                    if (pPlayer->uNumArmageddonCasts >= max_casts_a_day ||
                            pParty->armageddon_timer > 0) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
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
                    pCastSpell->uSpellID = SPELL_NONE;
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
                pPlayer->SetRecoveryTime((int64_t)(debug_combat_recovery_mul * (double)sRecoveryTime * flt_debugrecmod3));
            }
            pPlayer->PlaySound(SPEECH_CastSpell, 0);
        }
        if (spell_sound_flag) {
            pAudioPlayer->PlaySpellSound(pCastSpell->uSpellID, PID_INVALID);
        }

        pCastSpell->uSpellID = SPELL_NONE;
    }
}

/**
 * Add spell or skill into spell queue.
 * Spells from this queue will be cast in event queue processing.
 *
 * @offset 0x00427DA0
 */
static size_t pushCastSpellInfo(SPELL_TYPE uSpellID,
                                uint16_t uPlayerID,
                                int16_t skill_level,
                                SpellCastFlags uFlags,
                                int spell_sound_id) {
    // uFlags: ON_CAST_*
    for (size_t i = 0; i < pCastSpellInfo.size(); i++) {
        if (pCastSpellInfo[i].uSpellID == SPELL_NONE) {
            pCastSpellInfo[i].uSpellID = uSpellID;
            pCastSpellInfo[i].uPlayerID = uPlayerID;
            if (uFlags & ON_CAST_TargetIsParty) {
                pCastSpellInfo[i].uPlayerID_2 = uPlayerID;
            }
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

void CastSpellInfoHelpers::cancelSpellCastInProgress() {
    bool targeted_spell_canceled = false;

    for (CastSpellInfo &spellInfo : pCastSpellInfo) {
        if (spellInfo.uSpellID != SPELL_NONE &&
            spellInfo.uFlags & ON_CAST_CastingInProgress) {
            // Only one targeted spell can exist in queue.
            assert(!targeted_spell_canceled);

            spellInfo.uSpellID = SPELL_NONE;

            if (pGUIWindow_CastTargetedSpell) {
                // TODO: where object is deleted?
                pGUIWindow_CastTargetedSpell->Release();
                pGUIWindow_CastTargetedSpell = nullptr;
            }
            mouse->SetCursorImage("MICON1");
            GameUI_StatusBar_Update(true);
            IsEnchantingInProgress = false;
            back_to_game();

            targeted_spell_canceled = true;
        }
    }
}

void pushSpellOrRangedAttack(SPELL_TYPE spell,
                             unsigned int uPlayerID,
                             PLAYER_SKILL skill_value,
                             SpellCastFlags flags,
                             int a6) {
    // if (!pParty->bTurnBasedModeOn
    //  || (result = pTurnEngine->field_4, pTurnEngine->field_4 != 1) &&
    //  pTurnEngine->field_4 != 3 )
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage == TE_WAIT ||
            pTurnEngine->turn_stage == TE_MOVEMENT) {
            return;
        }
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
            case SPELL_AIR_LIGHTNING_BOLT:
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
                if (!a6) {
                    // These spells are targeted unless used from quick spell button
                    flags |= ON_CAST_TargetCrosshair;
                }
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
                if (!skill_value) {
                    skill_value = player->pActiveSkills[PLAYER_SKILL_SPIRIT];
                }
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_EXPERT && !engine->config->debug.AllMagic.Get()) {
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                }
                break;

            case SPELL_SPIRIT_PRESERVATION:
                if (!skill_value) {
                    skill_value = player->pActiveSkills[PLAYER_SKILL_SPIRIT];
                }
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_MASTER && !engine->config->debug.AllMagic.Get()) {
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                }
                break;

            case SPELL_DARK_PAIN_REFLECTION:
                if (!skill_value) {
                    skill_value = player->pActiveSkills[PLAYER_SKILL_DARK];
                }
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_MASTER && !engine->config->debug.AllMagic.Get()) {
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                }
                break;

            case SPELL_BODY_HAMMERHANDS:
                if (!skill_value) {
                    skill_value = player->pActiveSkills[PLAYER_SKILL_BODY];
                }
                if (GetSkillMastery(skill_value) < PLAYER_SKILL_MASTERY_GRANDMASTER && !engine->config->debug.AllMagic.Get()) {
                    flags |= ON_CAST_SinglePlayer_BigImprovementAnim;
                }
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

    // Any active targeted spell will be canceled in cancelSpellCastInProgress function.
    // But if we set uSpellID for current targeted spell to zero, that function will not release
    // pGUIWindow_CastTargetedSpell object, which can result in memory leak.
#if 0
    // clear previous casts
    if (flags & ON_CAST_CastingInProgress) {
        for (uint i = 0; i < CastSpellInfoCount; ++i) {
            if (pCastSpellInfo[i].uFlags & ON_CAST_CastingInProgress) {
                pCastSpellInfo[i].uSpellID = SPELL_NONE;
                break;
            }
        }
    }
#endif

    CastSpellInfoHelpers::cancelSpellCastInProgress();

    int result = pushCastSpellInfo(spell, uPlayerID, skill_value, flags, a6);

    // TODO: if no more place for spells in queue then spell is just ignored?
    //       Need assert?
    if (result != -1) {
        Sizei renDims = render->GetRenderDimensions();
        if (flags & ON_CAST_SinglePlayer_BigImprovementAnim) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({52, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 0, InputAction::SelectChar1);
            pGUIWindow_CastTargetedSpell->CreateButton({165, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 1, InputAction::SelectChar2);
            pGUIWindow_CastTargetedSpell->CreateButton({280, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 2, InputAction::SelectChar3);
            pGUIWindow_CastTargetedSpell->CreateButton({390, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_Character_Big_Improvement, 3, InputAction::SelectChar4);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_TargetCrosshair) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0,
                UIMSG_CastSpell_Shoot_Monster, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Telekenesis) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0,
                UIMSG_CastSpell_Telekinesis, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Enchantment) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = pCastSpellInfo[result].GetCastSpellInInventoryWindow();
            IsEnchantingInProgress = true;
            some_active_character = uActiveCharacter;
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_MonsterSparkles) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton({0x34u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 0, InputAction::SelectChar1);
            pGUIWindow_CastTargetedSpell->CreateButton({0xA5u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 1, InputAction::SelectChar2);
            pGUIWindow_CastTargetedSpell->CreateButton({0x118u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 2, InputAction::SelectChar3);
            pGUIWindow_CastTargetedSpell->CreateButton({0x186u, 0x1A6u}, {0x23u, 0}, 2, 0, UIMSG_CastSpell_Character_Small_Improvement, 3, InputAction::SelectChar4);
            pGUIWindow_CastTargetedSpell->CreateButton({8, 8}, {game_viewport_width, game_viewport_height}, 1, 0, UIMSG_CastSpell_Monster_Improvement, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
        }
        if (flags & ON_CAST_DarkSacrifice) {
            if (pGUIWindow_CastTargetedSpell) {
                return;
            }

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell({0, 0}, renDims, &pCastSpellInfo[result]);
            pBtn_NPCLeft = pGUIWindow_CastTargetedSpell->CreateButton({469, 178}, {ui_btn_npc_left->GetWidth(), ui_btn_npc_left->GetHeight()}, 1, 0,
                UIMSG_ScrollNPCPanel, 0, InputAction::Invalid, "", {ui_btn_npc_left});
            pBtn_NPCRight = pGUIWindow_CastTargetedSpell->CreateButton({626, 178}, {ui_btn_npc_right->GetWidth(), ui_btn_npc_right->GetHeight()}, 1, 0,
                UIMSG_ScrollNPCPanel, 1, InputAction::Invalid, "", {ui_btn_npc_right});
            pGUIWindow_CastTargetedSpell->CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_HiredNPC_CastSpell, 4, InputAction::SelectNPC1);
            pGUIWindow_CastTargetedSpell->CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_HiredNPC_CastSpell, 5, InputAction::SelectNPC2);
            // Next line was added to do something with picked item on Sacrifice cast
            pParty->PickedItem_PlaceInInventory_or_Drop();
        }
    }
}

void pushTempleSpell(SPELL_TYPE spell) {
    PLAYER_SKILL skill_value = ConstructSkillValue(PLAYER_SKILL_MASTERY_MASTER, pParty->uCurrentDayOfMonth % 7 + 1);

    pushSpellOrRangedAttack(spell, uActiveCharacter - 1, skill_value,
                            ON_CAST_TargetIsParty | ON_CAST_NoRecoverySpell, 0);
}

void pushNPCSpell(SPELL_TYPE spell) {
    pushSpellOrRangedAttack(spell, 0, SCROLL_OR_NPC_SPELL_SKILL_VALUE, 0, 0);
}

void pushScrollSpell(SPELL_TYPE spell, unsigned int uPlayerID) {
    pushSpellOrRangedAttack(spell, uPlayerID, SCROLL_OR_NPC_SPELL_SKILL_VALUE, ON_CAST_CastViaScroll, 0);
}
