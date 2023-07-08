#include "CastSpellInfo.h"

#include <cstring>
#include <vector>
#include <string>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Spells/Spells.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UISpell.h"
#include "GUI/UI/Books/LloydsBook.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"
#include "Library/Random/Random.h"

static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

static const size_t CAST_SPELL_QUEUE_SIZE = 10;
static std::array<CastSpellInfo, CAST_SPELL_QUEUE_SIZE> pCastSpellInfo;

CastSpellInfo::CastSpellInfo() {
    memset(this, 0, sizeof(*this));
}

/**
 * Common initialization of SpriteObject for spell casting
 *
 * Correct field uType of spritePtr must be set before calling this function
 * because initialization depends on it.
 */
static void initSpellSprite(SpriteObject *spritePtr,
                            int spellLevel,
                            CharacterSkillMastery spellMastery,
                            CastSpellInfo *pCastSpell) {
    assert(spritePtr && spritePtr->uType != SPRITE_NULL);
    assert(pCastSpell->uSpellID != SPELL_NONE);

    spritePtr->containing_item.Reset();
    spritePtr->spell_level = spellLevel;
    spritePtr->uSpellID = pCastSpell->uSpellID;
    spritePtr->spell_skill = spellMastery;
    spritePtr->uObjectDescID = pObjectList->ObjectIDByItemID(spritePtr->uType);
    spritePtr->spell_caster_pid = PID(OBJECT_Character, pCastSpell->uPlayerID);
    spritePtr->uSoundID = pCastSpell->sound_id;
}

/**
 * Notify that spell casting failed.
 */
static void spellFailed(CastSpellInfo *pCastSpell,
                        int error_str_id) {
    GameUI_SetStatusBar(error_str_id);
    pAudioPlayer->playUISound(SOUND_spellfail0201);
    pCastSpell->uSpellID = SPELL_NONE;
}

/**
 * Set recovery time for spell casting
 */
static void setSpellRecovery(CastSpellInfo *pCastSpell,
                             int recoveryTime) {
    if (pCastSpell->uFlags & ON_CAST_NoRecoverySpell) {
        return;
    }

    if (recoveryTime < 0) {
        recoveryTime = 0;
    }

    Character *pPlayer = &pParty->pCharacters[pCastSpell->uPlayerID];

    if (pParty->bTurnBasedModeOn) {
        pParty->pTurnBasedCharacterRecoveryTimes[pCastSpell->uPlayerID] = recoveryTime;

        pPlayer->SetRecoveryTime(recoveryTime);

        if (!enchantingActiveCharacter) {
            pTurnEngine->ApplyPlayerAction();
        }
    } else {
        pPlayer->SetRecoveryTime((int64_t)(debug_combat_recovery_mul * (double)recoveryTime * flt_debugrecmod3));
    }

    // It's here to set character portrain emotion on spell cast.
    // There's no actual spell speech.
    pPlayer->playReaction(SPEECH_CAST_SPELL);
}

void CastSpellInfoHelpers::castSpell() {
    CharacterSkillType which_skill;
    AIDirection target_direction;

    static const int ONE_THIRD_PI = TrigLUT.uIntegerPi / 3;

    for (CastSpellInfo &spellInfo : pCastSpellInfo) {  // cycle through spell queue
        SpriteObject pSpellSprite;
        CastSpellInfo *pCastSpell = &spellInfo;
        int uRequiredMana{}, recoveryTime{}, failureRecoveryTime{};

        if (pCastSpell->uSpellID == SPELL_NONE) {
            continue;  // spell item blank skip to next
        }

        if (pParty->Invisible()) {
            // casting a spell breaks invisibility
            pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (pCastSpell->uFlags & ON_CAST_CastingInProgress) {
            if (!pParty->pCharacters[pCastSpell->uPlayerID].CanAct()) {
                // this cancels the spell cast if the character can no longer act
                cancelSpellCastInProgress();
            }
            continue;
        }

        Character *pPlayer = &pParty->pCharacters[pCastSpell->uPlayerID];

        // Pid of target or 0 if spell was quick casted
        int spell_targeted_at = pCastSpell->spell_target_pid;

        // First try to pick live actor mouse is pointing at
        if (spell_targeted_at == 0 &&
                mouse->uPointingObjectID &&
                PID_TYPE(mouse->uPointingObjectID) == OBJECT_Actor &&
                pActors[PID_ID(mouse->uPointingObjectID)].CanAct()) {
            spell_targeted_at = mouse->uPointingObjectID;
        }

        // Otherwise pick closest live actor
        if (spell_targeted_at == 0) {
            bool target_undead;
            if (pCastSpell->uSpellID == SPELL_LIGHT_DESTROY_UNDEAD ||
                    pCastSpell->uSpellID == SPELL_SPIRIT_TURN_UNDEAD ||
                    pCastSpell->uSpellID == SPELL_DARK_CONTROL_UNDEAD) {
                target_undead = true;
            } else {
                target_undead = false;
            }

            spell_targeted_at = stru_50C198.FindClosestActor(engine->config->gameplay.RangedAttackDepth.value(), 1, target_undead);
        }

        pSpellSprite.uType = SpellSpriteMapping[pCastSpell->uSpellID];

        if (pSpellSprite.uType != SPRITE_NULL) {
            if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                int player_pid = PID(OBJECT_Character, pCastSpell->uPlayerID + 1);
                Actor::GetDirectionInfo(player_pid, spell_targeted_at, &target_direction, 0);  // target direciton
            } else {
                target_direction.uYawAngle = pParty->_viewYaw;  // spray infront of party
                target_direction.uPitchAngle = pParty->_viewPitch;
            }
        }

        int spell_level;
        CharacterSkillMastery spell_mastery;
        if (pCastSpell->forced_spell_skill_level) {
            // for spell scrolls - decode spell power and mastery
            spell_level = pCastSpell->forced_spell_skill_level.level();
            spell_mastery = pCastSpell->forced_spell_skill_level.mastery();
        } else {
            which_skill = getSkillTypeForSpell(pCastSpell->uSpellID);

            CombinedSkillValue val = pPlayer->getActualSkillValue(which_skill);
            spell_level = val.level();
            spell_mastery = val.mastery();

            if (engine->config->debug.AllMagic.value()) {
                spell_level = 10;
                spell_mastery = CHARACTER_SKILL_MASTERY_GRANDMASTER;
            }
        }

        if (isRegularSpell(pCastSpell->uSpellID)) {
            if (pCastSpell->forced_spell_skill_level || engine->config->debug.AllMagic.value()) {
                uRequiredMana = 0;
            } else {
                uRequiredMana = pSpellDatas[pCastSpell->uSpellID].mana_per_skill[std::to_underlying(spell_mastery) - 1];
            }

            recoveryTime = pSpellDatas[pCastSpell->uSpellID].recovery_per_skill[std::to_underlying(spell_mastery) - 1];
        }

        // Recovery time for spell failure if it cannot be cast at all in current context
        failureRecoveryTime = recoveryTime * engine->config->gameplay.SpellFailureRecoveryMod.value();

        if (!pCastSpell->forced_spell_skill_level) {
            if (which_skill == CHARACTER_SKILL_DARK && pParty->uCurrentHour == 0 && pParty->uCurrentMinute == 0 ||
                which_skill == CHARACTER_SKILL_LIGHT && pParty->uCurrentHour == 12 && pParty->uCurrentMinute == 0) {  // free spells at midnight or midday
                uRequiredMana = 0;
            }
        }

        if (isRegularSpell(pCastSpell->uSpellID) && !pPlayer->CanCastSpell(uRequiredMana)) {
            spellFailed(pCastSpell, LSTR_NOT_ENOUGH_SPELLPOINTS);
            continue;
        }

        if (!pCastSpell->forced_spell_skill_level // temple or NPC spells must not fail because of curse
                                                  // TODO(Nik-RE-dev): does scrolls must fail?
                && pPlayer->IsCursed()
                && isRegularSpell(pCastSpell->uSpellID)
                && grng->random(100) < 50) {  // character is cursed and have a chance to fail spell casting
            setSpellRecovery(pCastSpell, SPELL_FAILURE_RECOVERY_TIME_ON_CURSE);
            spellFailed(pCastSpell, LSTR_SPELL_FAILED);
            pPlayer->SpendMana(uRequiredMana);
            return;
        }

        assert(pCastSpell->uSpellID != SPELL_101 && "Unknown spell effect #101 (prolly flaming bow arrow");

        // First process special "pseudo" spells like bow or blaster shots
        // and spells that open additional menus like town portal or lloyd beacon
        if (pCastSpell->uSpellID == SPELL_BOW_ARROW) {
            int arrows = 1;
            if (spell_mastery >= CHARACTER_SKILL_MASTERY_MASTER) {
                arrows = 2;
            }

            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            if (pPlayer->WearsItem(ITEM_ARTIFACT_ULLYSES, ITEM_SLOT_BOW)) {
                pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(SPRITE_SPELL_WATER_ICE_BOLT);
            }
            pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
            setSpellRecovery(pCastSpell, pPlayer->GetAttackRecoveryTime(true));
        } else if (pCastSpell->uSpellID == SPELL_LASER_PROJECTILE) {
            initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
            // TODO(pskelton): was pParty->uPartyHeight / 2
            pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
            pSpellSprite.spell_target_pid = spell_targeted_at;
            pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
            pSpellSprite.uFacing = target_direction.uYawAngle;
            pSpellSprite.containing_item = pPlayer->pInventoryItemList[pPlayer->pEquipment.uMainHand - 1];
            // &pParty->pCharacters[pCastSpell->uPlayerID].spellbook.pDarkSpellbook.bIsSpellAvailable[36
            // *
            // pParty->pCharacters[pCastSpell->uPlayerID].pEquipment.uMainHand + 5], );
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
            // TODO(Nik-RE-dev): recovery time for melee used?
            setSpellRecovery(pCastSpell, pPlayer->GetAttackRecoveryTime(false));
        } else if (pCastSpell->uSpellID == SPELL_WATER_TOWN_PORTAL) {
            int success_chance_percent = 10 * spell_level;
            bool castSuccessful = true;
            if (spell_mastery != CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                if (pParty->GetRedOrYellowAlert()) {
                    spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                    castSuccessful = false;
                } else if (grng->random(100) >= success_chance_percent) {
                    spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                    // Mana was not spend before on failure.
                    pPlayer->SpendMana(uRequiredMana);
                    castSuccessful = false;
                }
            }
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_OnCastTownPortal, PID(OBJECT_Character, pCastSpell->uPlayerID), 0);
            pAudioPlayer->playSpellSound(pCastSpell->uSpellID, PID_INVALID);
        } else if (pCastSpell->uSpellID == SPELL_WATER_LLOYDS_BEACON) {
            if (pCurrentMapName == "d05.blv") {  // Arena
                spellFailed(pCastSpell, LSTR_SPELL_FAILED);
            } else {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_OnCastLloydsBeacon, pCastSpell->uPlayerID, spell_level);
                pCastSpell->uFlags |= ON_CAST_NoRecoverySpell;
            }
        } else {
            switch (pCastSpell->uSpellID) {
                case SPELL_FIRE_TORCH_LIGHT:
                {
                    int spell_power;
                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_power = 2;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_power = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            num_spikes = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            num_spikes = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            num_spikes = 7;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            num_spikes = 9;
                            break;
                        default:
                            assert(false);
                    }
                    int spikes_active = 0;
                    for (const SpriteObject &spriteObject : pSpriteObjects) {
                        if (spriteObject.uType &&
                                spriteObject.uSpellID == SPELL_FIRE_FIRE_SPIKE &&
                                spriteObject.spell_caster_pid == PID(OBJECT_Character, pCastSpell->uPlayerID)) {
                            ++spikes_active;
                        }
                    }
                    if (spikes_active > num_spikes) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = (short)target_direction.uYawAngle;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->_viewYaw, pParty->_viewPitch + 10, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_AIR_IMPLOSION:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height / 2);
                    pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                    Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    break;
                }

                case SPELL_EARTH_MASS_DISTORTION:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        pActors[monster_id].buffs[ACTOR_BUFF_MASS_DISTORTION]
                            .Apply(GameTime(pMiscTimer->uTotalTimeElapsed + 128), spell_mastery, 0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].pos;
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    }
                    break;
                }

                case SPELL_LIGHT_DESTROY_UNDEAD:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    // v730 = spell_targeted_at >> 3;
                    // HIDWORD(spellduration) = (int)&pActors[PID_ID(spell_targeted_at)];
                    int monster_id = PID_ID(spell_targeted_at);
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos;
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    int obj_id = pSpellSprite.Create(0, 0, 0, 0);
                    if (!MonsterStats::BelongsToSupertype(pActors[monster_id].monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    Actor::DamageMonsterFromParty(PID(OBJECT_Item, obj_id), monster_id, &spell_velocity);
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
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
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
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR ||
                            uCurrentlyLoadedLevelType == LEVEL_OUTDOOR &&
                            (pParty->uCurrentHour < 5 || pParty->uCurrentHour >= 21)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_LIGHT)) {
                        Actor::AI_Stand(monster_id, 4, 0x80, 0);
                        pActors[monster_id].buffs[ACTOR_BUFF_PARALYZED]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(3 * spell_level), spell_mastery, 0, 0, 0);
                        pActors[monster_id].attributes |= ACTOR_AGGRESSOR;
                        pActors[monster_id].speed.x = 0;
                        pActors[monster_id].speed.y = 0;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], Color()); // TODO(captainurist): why transparent black?
                    }
                    break;
                }

                case SPELL_EARTH_SLOW:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            spell_power = 2;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 2;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 4;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            spell_power = 8;
                            break;
                        default:
                            assert(false);
                    }
                    // v721 = 836 * PID_ID(spell_targeted_at);
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_EARTH)) {
                        pActors[monster_id].buffs[ACTOR_BUFF_SLOWED].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                        pActors[monster_id].attributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], Color()); // TODO(captainurist): why transparent black?
                    }
                    break;
                }

                case SPELL_MIND_CHARM:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                        // Wrong durations from vanilla fixed
                        GameTime spell_duration;

                        switch (spell_mastery) {
                            case CHARACTER_SKILL_MASTERY_EXPERT:
                                spell_duration = GameTime::FromMinutes(5 * spell_level);
                                break;
                            case CHARACTER_SKILL_MASTERY_MASTER:
                                spell_duration = GameTime::FromMinutes(10 * spell_level);
                                break;
                            case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                                // Time must be infinite until the player leaves the map
                                spell_duration = GameTime::FromYears(1);
                                break;
                            case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 have different durations
                            default:
                                assert(false);
                        }

                        pActors[monster_id].buffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_ENSLAVED].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_CHARM].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height);
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
                    // Spell level was multiplied by 300 (which number of seconds in 5 minutes)
                    // This calculation was moved to spell impact processing function
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
                    ItemGen *item = &pParty->pCharacters[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    item->UpdateTempBonus(pParty->GetPlayingTime());
                    if (item->uItemID == ITEM_BLASTER ||
                            item->uItemID == ITEM_BLASTER_RIFLE ||
                            item->IsBroken() ||
                            pItemTable->IsMaterialNonCommon(item) ||
                            item->special_enchantment != ITEM_ENCHANTMENT_NULL ||
                            item->uEnchantmentType != 0 ||
                            !item->isWeapon()) {
                        AfterEnchClickEventId = UIMSG_Escape;
                        AfterEnchClickEventSecondParam = 0;
                        AfterEnchClickEventTimeout = Timer::Second; // was 1, increased to make message readable
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }

                    switch (pCastSpell->uSpellID) {
                        case SPELL_FIRE_FIRE_AURA:
                            switch (spell_mastery) {
                                case CHARACTER_SKILL_MASTERY_NOVICE:
                                    item->special_enchantment = ITEM_ENCHANTMENT_OF_FIRE;
                                    break;
                                case CHARACTER_SKILL_MASTERY_EXPERT:
                                    item->special_enchantment = ITEM_ENCHANTMENT_OF_FLAME;
                                    break;
                                case CHARACTER_SKILL_MASTERY_MASTER:
                                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
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

                    if (spell_mastery < CHARACTER_SKILL_MASTERY_GRANDMASTER) {
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_power = 1;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_power = 1;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_power = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_power = 10;
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_REGENERATION]
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
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(4 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    bool has_weak = false;
                    for (const Character &character : pParty->pCharacters) {
                        if (character.conditions.Has(CONDITION_WEAK)) {
                            has_weak = true;
                        }
                    }
                    if (has_weak) {
                        // If any chanracter is in weak state, spell has no effect but mana is
                        // spent
                        // TODO: Vanilla bug here? See: https://www.celestialheavens.com/forum/10/7196
                        pPlayer->SpendMana(uRequiredMana);
                        setSpellRecovery(pCastSpell, recoveryTime);
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level + 1);
                            break;
                        default:
                            assert(false);
                    }

                    int spell_power = spell_level + 5;
                    if (spell_mastery == CHARACTER_SKILL_MASTERY_NOVICE) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_BLESS]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    } else {
                        for (size_t i = 0; i < pParty->pCharacters.size(); i++) {
                            pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_BLESS]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                        }
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    }
                    break;
                }

                case SPELL_SPIRIT_SPIRIT_LASH:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    float dist = (pActors[monster_id].pos - pParty->pos).toFloat().length();
                    if (dist <= 307.2) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = pActors[monster_id].pos - Vec3i(0, 0, pActors[monster_id].height * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, spell_targeted_at);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), monster_id, &spell_velocity);
                    } else {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
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
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
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
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        case CHARACTER_SKILL_MASTERY_EXPERT:
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
                    assert(spell_mastery >= CHARACTER_SKILL_MASTERY_MASTER);

                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_METEOR_SHOWER_INDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    ObjectType obj_type = PID_TYPE(spell_targeted_at);
                    Vec3i dist;
                    if (obj_type == OBJECT_Actor) {  // quick cast can specify target
                        dist = pActors[PID_ID(spell_targeted_at)].pos;
                    } else {
                        dist = pParty->pos + Vec3i(2048 * pCamera3D->_yawRotationCosine, 2048 * pCamera3D->_yawRotationSine, 0);
                    }
                    int j = 0, k = 0;
                    int yaw, pitch;
                    int meteor_num = (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) ? 20 : 16;
                    for (; meteor_num; meteor_num--) {
                        int originHeight = grng->random(1000);
                        // TODO(Nik-RE-dev): condition is always false
                        if (Vec3s(j, k, originHeight - 2500).length() <= 1.0) {
                            pitch = 0;
                            yaw = 0;
                        } else {
                            pitch = TrigLUT.atan2(sqrt(j * j + k * k), originHeight - 2500);
                            yaw = TrigLUT.atan2(j, k);
                        }
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = dist + Vec3i(0, 0, originHeight + 2500);
                        pSpellSprite.spell_target_pid = (obj_type == OBJECT_Actor) ? spell_targeted_at : 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = stru_50C198._427546(originHeight + 2500);
                        pSpellSprite.uFacing = yaw;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                        if (pSpellSprite.Create(yaw, pitch, spell_speed, 0) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        j = grng->random(1024) - 512;
                        k = grng->random(1024) - 512;
                    }
                    break;
                }

                case SPELL_FIRE_INFERNO:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_INFERNO_OUTDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        Vec3i spell_velocity = Vec3i(0, 0, 0);
                        pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), actor->id, &spell_velocity);
                        spell_fx_renderer->RenderAsSprite(&pSpellSprite);
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.OrangeyRed, 0x40);
                    break;
                }

                case SPELL_AIR_WIZARD_EYE:
                {
                    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, 0, 0, 0);
                    break;
                }

                case SPELL_AIR_FEATHER_FALL:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        default:
                            assert(false);
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            sparks_number = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            sparks_number = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            sparks_number = 7;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            sparks_number = 9;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_spray_angle_start = ONE_THIRD_PI / -2;
                    int spell_spray_angle_end = ONE_THIRD_PI / 2;
                    while (spell_spray_angle_start <= spell_spray_angle_end) {
                        pSpellSprite.uSpriteFrameID = grng->random(64); // TODO(captainurist): this doesn't look like a frame id initialization
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
                    if (pParty->isAirborne()) {
                        spellFailed(pCastSpell, LSTR_CANT_JUMP_AIRBORNE);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    pParty->uFlags |= PARTY_FLAGS_1_JUMPING;
                    pParty->speed.z = 1000;
                    pParty->pos.z += 5;
                    break;
                }

                case SPELL_AIR_INVISIBILITY:
                {
                    GameTime spell_duration;
                    int spell_power;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            spell_power = 3 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            spell_power = 4 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    if (pParty->GetRedOrYellowAlert()) {
                        spellFailed(pCastSpell, LSTR_HOSTILE_CREATURES_NEARBY);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_AIR_FLY:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_FLY_INDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    if (!pParty->pCharacters[pCastSpell->uPlayerID].GetMaxMana() && !engine->config->debug.AllMagic.value()) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].Apply(
                            pParty->GetPlayingTime() + GameTime::FromHours(spell_level),
                            spell_mastery, 0, 0,
                            pCastSpell->uPlayerID + 1);
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff = (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER);
                    break;
                }

                case SPELL_AIR_STARBURST:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_STARBURST_INDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    ObjectType obj_type = PID_TYPE(spell_targeted_at);
                    Vec3i dist;
                    if (obj_type == OBJECT_Actor) {  // quick cast can specify target
                        dist = pActors[PID_ID(spell_targeted_at)].pos;
                    } else {
                        dist = pParty->pos + Vec3i(2048 * pCamera3D->_yawRotationCosine, 2048 * pCamera3D->_yawRotationSine, 0);
                    }
                    int j = 0, k = 0;
                    int yaw, pitch;
                    for (int star_num = 20; star_num; star_num--) {
                        int originHeight = grng->random(1000);
                        // TODO(Nik-RE-dev): condition is always false
                        if (Vec3s(j, k, originHeight - 2500).length() <= 1.0) {
                            pitch = 0;
                            yaw = 0;
                        } else {
                            pitch = TrigLUT.atan2(sqrt(j * j + k * k), originHeight - 2500);
                            yaw = TrigLUT.atan2(j, k);
                        }
                        initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                        pSpellSprite.vPosition = dist + Vec3i(0, 0, originHeight + 2500);
                        pSpellSprite.spell_target_pid = (obj_type == OBJECT_Actor) ? spell_targeted_at : 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = stru_50C198._427546(originHeight + 2500);
                        pSpellSprite.uFacing = yaw;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                        }
                        int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                        if (pSpellSprite.Create(yaw, pitch, spell_speed, 0) != -1 &&
                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        j = grng->random(1024) - 512;
                        k = grng->random(1024) - 512;
                    }
                    break;
                }

                case SPELL_WATER_AWAKEN:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    for (Character &character : pParty->pCharacters) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            if (character.conditions.Has(CONDITION_SLEEP)) {
                                character.conditions.Reset(CONDITION_SLEEP);
                                character.playReaction(SPEECH_AWAKEN);
                            }
                        } else {
                            if (character.DiscardConditionIfLastsLongerThan(CONDITION_SLEEP, pParty->GetPlayingTime() - spell_duration)) {
                                character.playReaction(SPEECH_AWAKEN);
                            }
                        }
                    }
                    break;
                }

                case SPELL_WATER_POISON_SPRAY:
                {
                    int shots_num;
                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            shots_num = 1;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            shots_num = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            shots_num = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            shots_num = 7;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
                    if (!pParty->pCharacters[pCastSpell->uPlayerID].GetMaxMana() && !engine->config->debug.AllMagic.value()) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, pCastSpell->uPlayerID + 1);
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff = (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER);
                    break;
                }

                case SPELL_WATER_RECHARGE_ITEM:
                {
                    ItemGen *item = &pParty->pCharacters[pCastSpell->uPlayerID_2].pInventoryItemList[pCastSpell->spell_target_pid];
                    if (!item->isWand() || item->IsBroken()) {
                        AfterEnchClickEventId = UIMSG_Escape;
                        AfterEnchClickEventSecondParam = 0;
                        AfterEnchClickEventTimeout = Timer::Second; // was 1, increased to make message readable
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }

                    double spell_recharge_factor;
                    if (spell_mastery == CHARACTER_SKILL_MASTERY_NOVICE || spell_mastery == CHARACTER_SKILL_MASTERY_EXPERT) {
                        spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.5;  // 50 %
                    } else if (spell_mastery == CHARACTER_SKILL_MASTERY_MASTER) {
                        spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.69999999;  // 30 %
                    } else if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
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
                        setSpellRecovery(pCastSpell, recoveryTime);
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
                    int rnd = grng->random(100);
                    pPlayer = &pParty->pCharacters[pCastSpell->uPlayerID_2];
                    ItemGen *spell_item_to_enchant = &pPlayer->pInventoryItemList[pCastSpell->spell_target_pid];
                    ITEM_EQUIP_TYPE this_equip_type = pItemTable->pItems[spell_item_to_enchant->uItemID].uEquipType;

                    // refs
                    // https://www.gog.com/forum/might_and_magic_series/a_little_enchant_item_testing_in_mm7
                    // http://www.pottsland.com/mm6/enchant.shtml
                    // also see STDITEMS.tx and SPCITEMS.txt in Events.lod

                    if ((spell_mastery == CHARACTER_SKILL_MASTERY_NOVICE || spell_mastery == CHARACTER_SKILL_MASTERY_EXPERT)) {
                        __debugbreak(); // SPELL_WATER_ENCHANT_ITEM is a master level spell
                    }

                    if ((spell_mastery == CHARACTER_SKILL_MASTERY_MASTER || spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) &&
                            isRegular(spell_item_to_enchant->uItemID) &&
                            spell_item_to_enchant->special_enchantment == ITEM_ENCHANTMENT_NULL &&
                            spell_item_to_enchant->uEnchantmentType == 0 &&
                            spell_item_to_enchant->m_enchantmentStrength == 0 &&
                            !spell_item_to_enchant->IsBroken()) {
                        // break items with low value
                        if ((spell_item_to_enchant->GetValue() < 450 && !isWeapon(this_equip_type)) ||  // not weapons
                                (spell_item_to_enchant->GetValue() < 250 && isWeapon(this_equip_type))) {  // weapons
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
                                if (rnd < 80 && isPassiveEquipment(this_equip_type)) { // chance to roll standard enchantment on non-weapons
                                    int ench_found = 0;
                                    int to_item_apply_sum = 0;
                                    int ench_array[100] = { 0 };

                                    // finds how many possible enchaments and adds up to item apply values
                                    // if (pItemTable->pEnchantments_count > 0) {
                                    for (int norm_ench_loop = 0; norm_ench_loop < 24; ++norm_ench_loop) {
                                        const std::string &bonusStat = pItemTable->standardEnchantments[norm_ench_loop].pBonusStat;
                                        if (!bonusStat.empty()) {
                                            int this_to_apply = pItemTable->standardEnchantments[norm_ench_loop].chancesByItemType[this_equip_type];
                                            to_item_apply_sum += this_to_apply;
                                            if (this_to_apply) {
                                                ench_array[ench_found] = norm_ench_loop;
                                                ench_found++;
                                            }
                                        }
                                    }
                                    // }

                                    // pick a random ench
                                    int item_apply_rand = grng->random(to_item_apply_sum);
                                    int target_item_apply_rand = item_apply_rand + 1;
                                    int current_item_apply_sum = 0;
                                    int step = 0;

                                    // step through until we hit that ench
                                    for (step = 0; step < ench_found; step++) {
                                        current_item_apply_sum += pItemTable->standardEnchantments[ench_array[step]].chancesByItemType[this_equip_type];
                                        if (current_item_apply_sum >= target_item_apply_rand) {
                                            break;
                                        }
                                    }

                                    // assign ench and power
                                    spell_item_to_enchant->uEnchantmentType = (ITEM_ENCHANTMENT)ench_array[step];

                                    int ench_power = 0;
                                    // master 3-8  - guess work needs checking
                                    if (spell_mastery == CHARACTER_SKILL_MASTERY_MASTER) ench_power = grng->random(6) + 3;
                                    // gm 6-12   - guess work needs checking
                                    if (spell_mastery== CHARACTER_SKILL_MASTERY_GRANDMASTER) ench_power = grng->random(7) + 6;

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
                                            const std::string &bonusStatement = pItemTable->pSpecialEnchantments[spec_ench_loop].pBonusStatement;
                                            if (!bonusStatement.empty()) {
                                                if (pItemTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel == 3) {
                                                    continue;
                                                }
                                                if (spell_mastery == CHARACTER_SKILL_MASTERY_MASTER && (pItemTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel != 0)) {
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
                                    int item_apply_rand = grng->random(to_item_apply_sum);
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
                        pParty->pCharacters[pCastSpell->uPlayerID_2].playReaction(SPEECH_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    break;
                }

                case SPELL_EARTH_STONE_TO_FLESH:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_PETRIFIED)) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_PETRIFIED);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_PETRIFIED, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_EARTH_ROCK_BLAST:
                {
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = pParty->_viewYaw;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->_viewYaw, pParty->_viewPitch, spell_speed, pCastSpell->uPlayerID + 1) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_EARTH_DEATH_BLOSSOM:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    pSpellSprite.uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.uFacing = (short)pParty->_viewYaw;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= SPRITE_HALT_TURN_BASED;
                    }
                    int spell_speed = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(pParty->_viewYaw, TrigLUT.uIntegerHalfPi / 2, spell_speed, 0) != -1 &&
                            pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    break;
                }

                case SPELL_SPIRIT_DETECT_LIFE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(30 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_power = spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_power = 2 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_power = 4 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_power = 6 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    // LODWORD(spellduration) = 300;
                    if (pCastSpell->spell_target_pid == 0) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_FATE]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(5), spell_mastery, spell_power, 0, 0);
                    } else if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        int monster_id = PID_ID(pCastSpell->spell_target_pid);
                        pActors[monster_id].buffs[ACTOR_BUFF_FATE]
                            .Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(5), spell_mastery, spell_power, 0, 0);
                        pActors[monster_id].attributes |= ACTOR_AGGRESSOR;
                        spell_fx_renderer->sparklesOnActorAfterItCastsBuff(&pActors[monster_id], Color()); // TODO(captainurist): why transparent black?
                    }
                    break;
                }

                case SPELL_SPIRIT_REMOVE_CURSE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_CURSED)) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_CURSED);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_CURSED, pParty->GetPlayingTime() - spell_duration);
                        }
                        if (!pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_CURSED)) {
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        }
                    }
                    break;
                }

                case SPELL_SPIRIT_PRESERVATION:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }

                    if (spell_mastery == CHARACTER_SKILL_MASTERY_NOVICE || spell_mastery == CHARACTER_SKILL_MASTERY_EXPERT) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_PRESERVATION]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    } else {
                        for (Character &character : pParty->pCharacters) {
                            character.pCharacterBuffs[CHARACTER_BUFF_PRESERVATION]
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 + 1 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            // Was "3m + 1m * spell_level"
                            spell_duration = GameTime::FromMinutes(3 + 3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(3 + 5 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_SPIRIT_TURN_UNDEAD_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        if (MonsterStats::BelongsToSupertype(actor->monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                            pSpellSprite.Create(0, 0, 0, 0);
                            actor->buffs[ACTOR_BUFF_AFRAID].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.White, 192);
                    break;
                }

                case SPELL_SPIRIT_RAISE_DEAD:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT: // MM6 only
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_DEAD)) {
                        pParty->pCharacters[pCastSpell->uPlayerID_2].health = 1;
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_DEAD);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_UNCONSCIOUS);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_DEAD, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_UNCONSCIOUS, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pCharacters[pCastSpell->uPlayerID_2].SetCondition(CONDITION_WEAK, 0);
                    }
                    break;
                }

                case SPELL_SPIRIT_SHARED_LIFE:
                {
                    int shared_life_count;
                    if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                        shared_life_count = 4 * spell_level;
                    } else {
                        shared_life_count = 3 * spell_level;
                    }
                    int active_pl_num = 0;
                    for (const Character &character : pParty->pCharacters) {
                        if (character.conditions.HasNone({CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
                            shared_life_count += character.health;
                            active_pl_num++;
                        }
                    }
                    int mean_life = shared_life_count / active_pl_num;
                    for (size_t i = 0; i < pParty->pCharacters.size(); i++) {
                        if (pParty->pCharacters[i].conditions.HasNone({CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
                            pParty->pCharacters[i].health = mean_life;
                            if (pParty->pCharacters[i].health > pParty->pCharacters[i].GetMaxHealth()) {
                                pParty->pCharacters[i].health = pParty->pCharacters[i].GetMaxHealth();
                            }
                            if (pParty->pCharacters[i].health > 0) {
                                pParty->pCharacters[i].SetUnconcious(GameTime(0));
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
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT: // MM6 only
                            // Was 3 hours per spell level
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER: // MM6 only
                            // Was 3 days per spell level
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.HasAny({CONDITION_ERADICATED, CONDITION_DEAD})) {
                        if (!pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_WEAK)) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].playReaction(SPEECH_WEAK);
                        }
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_ERADICATED);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_DEAD);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_UNCONSCIOUS);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_ERADICATED, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_DEAD, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_UNCONSCIOUS, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pCharacters[pCastSpell->uPlayerID_2].SetCondition(CONDITION_WEAK, 1);
                        pParty->pCharacters[pCastSpell->uPlayerID_2].health = 1;
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    break;
                }

                case SPELL_MIND_CURE_PARALYSIS:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_PARALYZED)) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_PARALYZED);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_PARALYZED, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_MIND_REMOVE_FEAR:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_FEAR)) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_FEAR);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_FEAR, pParty->GetPlayingTime() - spell_duration);
                        }
                    }

                    break;
                }

                case SPELL_MIND_TELEPATHY:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    {
                        int monster_id = PID_ID(spell_targeted_at);
                        if (!pActors[monster_id].ActorHasItem()) {
                            pActors[monster_id].SetRandomGoldIfTheresNoItem();
                        }
                        int gold_num = 0;
                        if (pActors[monster_id].items[3].uItemID != ITEM_NULL) {
                            if (pItemTable->pItems[pActors[monster_id].items[3].uItemID].uEquipType == EQUIP_GOLD) {
                                gold_num = pActors[monster_id].items[3].special_enchantment;
                            }
                        }
                        ItemGen item;
                        item.Reset();
                        if (pActors[monster_id].carriedItemId != ITEM_NULL) {
                            item.uItemID = pActors[monster_id].carriedItemId;
                        } else {
                            for (const ItemGen &actorItem : pActors[monster_id].items) {
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
                        pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height);
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
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(10 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            // TODO: is this correct?
                            // Spell description says that spell effect is infinite.
                            spell_duration = GameTime::FromHours(1);
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    // v730 = 836 * monster_id;
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                        pActors[monster_id].buffs[ACTOR_BUFF_CHARM].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_ENSLAVED].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_BERSERK]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                        pActors[monster_id].monsterInfo.uHostilityType = MonsterInfo::Hostility_Long;
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    break;
                }

                case SPELL_MIND_ENSLAVE:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    GameTime spell_duration = GameTime::FromMinutes(10 * spell_level);
                    int monster_id = PID_ID(spell_targeted_at);
                    // v730 = 836 * monster_id;
                    if (MonsterStats::BelongsToSupertype(pActors[monster_id].monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    if (pActors[monster_id].DoesDmgTypeDoDamage(DMGT_MIND)) {
                        pActors[monster_id].buffs[ACTOR_BUFF_BERSERK].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_CHARM].Reset();
                        pActors[monster_id].buffs[ACTOR_BUFF_ENSLAVED]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    break;
                }

                case SPELL_MIND_MASS_FEAR:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                        case CHARACTER_SKILL_MASTERY_EXPERT: // MM6 only
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        default:
                            assert(false);
                    }

                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_MIND_MASS_FEAR_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        // Change: do not exit loop when first undead monster is found
                        if (!MonsterStats::BelongsToSupertype(actor->monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                            pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
                            pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                            pSpellSprite.Create(0, 0, 0, 0);
                            if (actor->DoesDmgTypeDoDamage(DMGT_MIND)) {
                                actor->buffs[ACTOR_BUFF_AFRAID].Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                            }
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Night, 192);
                    break;
                }

                case SPELL_MIND_CURE_INSANITY:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT: // MM6 only
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_INSANE)) {
                        if (!pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_WEAK)) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].playReaction(SPEECH_WEAK);
                        }
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_INSANE);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_INSANE, pParty->GetPlayingTime() - spell_duration);
                        }
                        pParty->pCharacters[pCastSpell->uPlayerID_2].SetCondition( CONDITION_WEAK, 0);
                    }
                    break;
                }

                case SPELL_EARTH_TELEKINESIS:
                {
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    int obj_id = PID_ID(spell_targeted_at);
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Item) {
                        if (pItemTable->pItems[pSpriteObjects[obj_id].containing_item.uItemID].uEquipType == EQUIP_GOLD) {
                            pParty->partyFindsGold(pSpriteObjects[obj_id].containing_item.special_enchantment, GOLD_RECEIVE_SHARE);
                        } else {
                            GameUI_SetStatusBar(LSTR_FMT_YOU_FOUND_ITEM, pItemTable->pItems[pSpriteObjects[obj_id].containing_item.uItemID].pUnidentifiedName.c_str());
                            if (!pParty->addItemToParty(&pSpriteObjects[obj_id].containing_item)) {
                                pParty->setHoldingItem(&pSpriteObjects[obj_id].containing_item);
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
                            eventProcessor(pLevelDecorations[obj_id].uEventID, spell_targeted_at, 1);
                        }
                        // TODO(captainurist): investigate, that's a very weird std::to_underlying call.
                        if (pLevelDecorations[std::to_underlying(pSpriteObjects[obj_id].containing_item.uItemID)].IsInteractive()) {
                            activeLevelDecoration = &pLevelDecorations[obj_id];
                            eventProcessor(engine->_persistentVariables.decorVars[pLevelDecorations[obj_id].eventVarId] + 380, 0, 1);
                            activeLevelDecoration = nullptr;
                        }
                    }
                    if (PID_TYPE(spell_targeted_at) == OBJECT_Face) {
                        int event;
                        OpenedTelekinesis = true;
                        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                            event = pIndoor->pFaceExtras[pIndoor->pFaces[obj_id].uFaceExtraID].uEventID;
                        } else {
                            event = pOutdoor->pBModels[spell_targeted_at >> 9].pFaces[obj_id & 0x3F].sCogTriggeredID;
                        }
                        eventProcessor(event, spell_targeted_at, 1);
                    }
                    break;
                }

                case SPELL_BODY_CURE_WEAKNESS:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_WEAK)) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_WEAK);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_WEAK, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_BODY_FIRST_AID:
                {
                    int heal_amount;
                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            heal_amount = 2 * spell_level + 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            heal_amount = 3 * spell_level + 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            heal_amount = 4 * spell_level + 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            heal_amount = 5 * spell_level + 5;
                            break;
                        default:
                            assert(false);
                    }
                    if (pCastSpell->spell_target_pid == PID_INVALID) {
                        pParty->pCharacters[pCastSpell->uPlayerID_2].Heal(heal_amount);
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    }
                    if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                        int monster_id = PID_ID(pCastSpell->spell_target_pid);
                        if (pActors[monster_id].aiState != Dead &&
                            pActors[monster_id].aiState != Dying &&
                            pActors[monster_id].aiState != Disabled &&
                            pActors[monster_id].aiState != Removed) {
                            pActors[monster_id].currentHP += heal_amount;
                            if (pActors[monster_id].currentHP > pActors[monster_id].monsterInfo.uHP) {
                                pActors[monster_id].currentHP = pActors[monster_id].monsterInfo.uHP;
                            }
                        }
                    }
                    break;
                }

                case SPELL_BODY_CURE_POISON:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.HasAny({CONDITION_POISON_WEAK, CONDITION_POISON_MEDIUM, CONDITION_POISON_SEVERE})) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_POISON_WEAK);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_POISON_MEDIUM);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_POISON_SEVERE);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_POISON_WEAK, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_POISON_MEDIUM, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_POISON_SEVERE, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_BODY_CURE_DISEASE:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 only
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT: // MM6 only
                            spell_duration = GameTime::FromHours(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromDays(spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime(0);
                            break;
                        default:
                            assert(false);
                    }

                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.HasAny({CONDITION_DISEASE_WEAK, CONDITION_DISEASE_MEDIUM, CONDITION_DISEASE_SEVERE})) {
                        if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_DISEASE_WEAK);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_DISEASE_MEDIUM);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Reset(CONDITION_DISEASE_SEVERE);
                        } else {
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_DISEASE_WEAK, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_DISEASE_MEDIUM, pParty->GetPlayingTime() - spell_duration);
                            pParty->pCharacters[pCastSpell->uPlayerID_2]
                                .DiscardConditionIfLastsLongerThan(CONDITION_DISEASE_SEVERE, pParty->GetPlayingTime() - spell_duration);
                        }
                    }
                    break;
                }

                case SPELL_BODY_PROTECTION_FROM_MAGIC:
                {
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, 0, 0);
                    break;
                }

                case SPELL_BODY_HAMMERHANDS:
                {
                    if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                        for (Character &character : pParty->pCharacters) {
                            character.pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS]
                                .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, spell_level, 0);
                        }
                    } else {
                    spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS]
                        .Apply(pParty->GetPlayingTime() + GameTime::FromHours(spell_level), spell_mastery, spell_level, spell_level, 0);
                    }
                    break;
                }

                case SPELL_BODY_POWER_CURE:
                {
                    for (Character &character : pParty->pCharacters) {
                        character.Heal(5 * spell_level + 10);
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    break;
                }

                case SPELL_LIGHT_DISPEL_MAGIC:
                {
                    recoveryTime -= spell_level;
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_DISPEL_MAGIC_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    // Spell damage processing was removed because Dispel Magic does not do damage
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        pSpellSprite.Create(0, 0, 0, 0);
                        for (SpellBuff &buff : actor->buffs) {
                            buff.Reset();
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.VibrantGreen, 192);
                    break;
                }

                case SPELL_LIGHT_SUMMON_ELEMENTAL:
                {
                    GameTime spell_duration;
                    int max_summoned;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            max_summoned = 1;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(15 * spell_level);
                            max_summoned = 3;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromMinutes(15 * spell_level);
                            max_summoned = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int mon_num = 0;
                    for (const Actor &actor : pActors) {
                        if (actor.aiState != Dead &&
                            actor.aiState != Removed &&
                            actor.aiState != Disabled &&
                                PID(OBJECT_Character, pCastSpell->uPlayerID) == actor.summonerId) {
                            ++mon_num;
                        }
                    }
                    if (mon_num >= max_summoned) {
                        spellFailed(pCastSpell, LSTR_SUMMONS_LIMIT_REACHED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
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
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromHours(3 * spell_level);
                            spell_power = 3 * spell_level + 10;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(4 * spell_level);
                            spell_power = 4 * spell_level + 10;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(5 * spell_level);
                            spell_power = 5 * spell_level + 10;
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE: // MM6 have different durations
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    break;
                }

                case SPELL_LIGHT_PRISMATIC_LIGHT:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_PRISMATIC_OUTDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    for (Actor *actor : render->getActorsInViewport(4096)) {
                        pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
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
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(4 * spell_level);
                            spell_power = 4 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(5 * spell_level);
                            spell_power = 5 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE: // In MM6 this spell is different and is of dark magic
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
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
                    int target_skill_level = spell_level + 5;
                    GameTime haste_duration, other_duration;
                    int target_spell_level;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            // Was "1h * spell_level + 1m" for other buffs
                            target_spell_level = spell_level * 4;
                            haste_duration = GameTime::FromHours(1).AddMinutes(3 * target_spell_level);
                            other_duration = GameTime::FromHours(1).AddMinutes(15 * target_spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
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
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                        default:
                            assert(false);
                    }
                    bool player_weak = false;
                    for (Character &character : pParty->pCharacters) {
                       character.pCharacterBuffs[CHARACTER_BUFF_BLESS]
                            .Apply(pParty->GetPlayingTime() + other_duration, spell_mastery, target_skill_level, 0, 0);
                        if (character.conditions.Has(CONDITION_WEAK)) {
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
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    for (Character &character : pParty->pCharacters) {
                        character.conditions.ResetAll();
                        character.health = character.GetMaxHealth();
                        character.mana = character.GetMaxMana();
                    }
                    spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    if (pPlayer->sAgeModifier + 10 >= 120) {
                        pPlayer->sAgeModifier = 120;
                    } else {
                        pPlayer->sAgeModifier = pPlayer->sAgeModifier + 10;
                    }
                    recoveryTime = 5 * spell_level;
                    ++pPlayer->uNumDivineInterventionCastsThisDay;
                    break;
                }

                case SPELL_DARK_REANIMATE:
                {
                    int target_monster_level;
                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            target_monster_level = 2 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            target_monster_level = 3 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            target_monster_level = 4 * spell_level;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            target_monster_level = 5 * spell_level;
                            break;
                        default:
                            assert(false);
                    }
                    int zombie_hp_limit = target_monster_level * 10;
                    if (pCastSpell->spell_target_pid == PID_INVALID) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        if (pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Has(CONDITION_DEAD)) {
                            pParty->pCharacters[pCastSpell->uPlayerID_2].SetCondition(CONDITION_ZOMBIE, 1);
                            GameUI_ReloadPlayerPortraits(pCastSpell->uPlayerID_2, (pParty->pCharacters[pCastSpell->uPlayerID_2].GetSexByVoice() != 0) + 23);
                            pParty->pCharacters[pCastSpell->uPlayerID_2].conditions.Set(CONDITION_ZOMBIE, pParty->GetPlayingTime());
                            // TODO: why call SetCondition and then conditions.Set?
                        }
                        break;
                    }
                    int monster_id = PID_ID(pCastSpell->spell_target_pid);
                    if (monster_id == -1) {
                        spellFailed(pCastSpell, LSTR_NO_VALID_SPELL_TARGET);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    if (pActors[monster_id].currentHP > 0 || pActors[monster_id].aiState != Dead && pActors[monster_id].aiState != Dying) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    // ++pSpellSprite.uType;
                    pSpellSprite.uType = SPRITE_SPELL_DARK_REANIMATE_1;
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos - Vec3i(0, 0, pActors[monster_id].height * -0.8);
                    pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                    pSpellSprite.Create(0, 0, 0, 0);
                    if (pActors[monster_id].monsterInfo.uLevel > target_monster_level) {
                        // Spell is still considered casted successfully, only monster level
                        // is too high to actually ressurect it.
                        break;
                    }
                    Actor::resurrect(monster_id);
                    if (pActors[monster_id].currentHP > zombie_hp_limit) {
                        pActors[monster_id].currentHP = zombie_hp_limit;
                    }
                    break;
                }

                case SPELL_DARK_SHARPMETAL:
                {
                    int blades_cound;
                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                            blades_cound = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            blades_cound = 5;
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            blades_cound = 7;
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            blades_cound = 9;
                            break;
                        default:
                            assert(false);
                    }
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    // TODO(pskelton): was pParty->uPartyHeight / 2
                    pSpellSprite.vPosition = pParty->pos + Vec3i(0, 0, pParty->height / 3);
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
                    // Vanilla behaviour changed:
                    // before if spell was targeted wrong, mana was spend on spell failure
                    if (!spell_targeted_at || PID_TYPE(spell_targeted_at) != OBJECT_Actor) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }

                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                            spell_duration = GameTime::FromMinutes(3 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromMinutes(5 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            // Time must be infinite until the player leaves the map
                            spell_duration = GameTime::FromYears(1);
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int monster_id = PID_ID(spell_targeted_at);
                    if (!MonsterStats::BelongsToSupertype(pActors[monster_id].monsterInfo.uID, MONSTER_SUPERTYPE_UNDEAD)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    if (!pActors[monster_id].DoesDmgTypeDoDamage(DMGT_DARK)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }
                    pActors[monster_id].buffs[ACTOR_BUFF_BERSERK].Reset();
                    pActors[monster_id].buffs[ACTOR_BUFF_CHARM].Reset();
                    pActors[monster_id].buffs[ACTOR_BUFF_ENSLAVED]
                        .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, 0, 0, 0);
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    pSpellSprite.vPosition = pActors[monster_id].pos + Vec3i(0, 0, pActors[monster_id].height);
                    pSpellSprite.uSectorID = pIndoor->GetSector(pSpellSprite.vPosition);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= SPRITE_ATTACHED_TO_HEAD;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                    break;
                }

                case SPELL_DARK_SACRIFICE:
                {
                    if (bNoNPCHiring) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }

                    FlatHirelings buf;
                    buf.Prepare();

                    int flatHirelingId = pParty->hirelingScrollPosition + pCastSpell->uPlayerID_2;

                    if (buf.IsFollower(flatHirelingId)) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        pPlayer->SpendMana(uRequiredMana); // decrease mana on failure
                        setSpellRecovery(pCastSpell, recoveryTime);
                        continue;
                    }

                    NPCData *npcData = buf.Get(flatHirelingId);
                    npcData->dialogue_1_evt_id = 1;
                    npcData->dialogue_2_evt_id = 0;
                    npcData->dialogue_3_evt_id = pIconsFrameTable->GetIcon("spell96")->GetAnimLength();
                    for (Character &character : pParty->pCharacters) {
                        character.health = character.GetMaxHealth();
                        character.mana = character.GetMaxMana();
                    }
                    LocationInfo *ddm_dlv = &currentLocationInfo();
                    ddm_dlv->reputation += 15;
                    if (ddm_dlv->reputation > 10000) {
                        ddm_dlv->reputation = 10000;
                    }
                    break;
                }

                case SPELL_DARK_PAIN_REFLECTION:
                {
                    GameTime spell_duration;

                    switch (spell_mastery) {
                        case CHARACTER_SKILL_MASTERY_EXPERT:
                        case CHARACTER_SKILL_MASTERY_MASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(5 * spell_level);
                        case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                            spell_duration = GameTime::FromHours(1).AddMinutes(15 * spell_level);
                            break;
                        case CHARACTER_SKILL_MASTERY_NOVICE:
                        default:
                            assert(false);
                    }
                    int spell_power = spell_level + 5;
                    if (spell_mastery != CHARACTER_SKILL_MASTERY_MASTER && spell_mastery != CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                        spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                        pParty->pCharacters[pCastSpell->uPlayerID_2].pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION]
                            .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                    } else {
                        for (Character &character : pParty->pCharacters) {
                            character.pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION]
                                .Apply(pParty->GetPlayingTime() + spell_duration, spell_mastery, spell_power, 0, 0);
                        }
                        spell_fx_renderer->SetPartyBuffAnim(pCastSpell->uSpellID);
                    }
                    break;
                }

                case SPELL_DARK_SOULDRINKER:
                {
                    initSpellSprite(&pSpellSprite, spell_level, spell_mastery, pCastSpell);
                    Vec3i spell_velocity = Vec3i(0, 0, 0);
                    std::vector<Actor*> actorsInViewport = render->getActorsInViewport(pCamera3D->GetMouseInfoDepth());
                    for (Actor *actor : actorsInViewport) {
                        pSpellSprite.vPosition = actor->pos - Vec3i(0, 0, actor->height * -0.8);
                        pSpellSprite.spell_target_pid = PID(OBJECT_Actor, actor->id);
                        Actor::DamageMonsterFromParty(PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)), actor->id, &spell_velocity);
                    }
                    int drained_health = (actorsInViewport.size() * (7 * spell_level + 25));
                    int active_pl_num = 0;
                    for (const Character &character : pParty->pCharacters) {
                        if (character.CanAct()) {
                            active_pl_num++;
                        }
                    }
                    for (size_t i = 0; i < pParty->pCharacters.size(); i++) {
                        if (pParty->pCharacters[i].CanAct()) {
                            pParty->pCharacters[i].health += drained_health / active_pl_num;
                            if (pParty->pCharacters[i].health > pParty->pCharacters[i].GetMaxHealth()) {
                                pParty->pCharacters[i].health = pParty->pCharacters[i].GetMaxHealth();
                            }
                            spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, i);
                        }
                    }
                    spell_fx_renderer->FadeScreen__like_Turn_Undead_and_mb_Armageddon(colorTable.Black, 64);
                    break;
                }

                case SPELL_DARK_ARMAGEDDON:
                {
                    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                        spellFailed(pCastSpell, LSTR_CANT_ARMAGEDDON_INDOORS);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    int max_casts_a_day;
                    if (spell_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                        max_casts_a_day = 4;
                    } else {
                        max_casts_a_day = 3;
                    }
                    if (pPlayer->uNumArmageddonCasts >= max_casts_a_day ||
                            pParty->armageddon_timer > 0) {
                        spellFailed(pCastSpell, LSTR_SPELL_FAILED);
                        setSpellRecovery(pCastSpell, failureRecoveryTime);
                        continue;
                    }
                    pParty->armageddon_timer = 256;
                    pParty->armageddonDamage = spell_level;
                    pParty->armageddonForceCount = 60;
                    ++pPlayer->uNumArmageddonCasts;
                    if (pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    for (uint i = 0; i < 50; i++) {
                        int rand_x = grng->random(4096) - 2048;
                        int rand_y = grng->random(4096) - 2048;
                        bool bOnWater = false;
                        int terr_height = GetTerrainHeightsAroundParty2(rand_x + pParty->pos.x, rand_y + pParty->pos.y, &bOnWater, 0);
                        SpriteObject::dropItemAt(SPRITE_SPELL_EARTH_ROCK_BLAST,
                                                 {rand_x + pParty->pos.x, rand_y + pParty->pos.y, terr_height + 16}, grng->random(500) + 500);
                    }
                    break;
                }
                default:
                    pCastSpell->uSpellID = SPELL_NONE;
                    assert(false && "All spells casted by player must be processed.");
                    continue;
            }

            pPlayer->SpendMana(uRequiredMana);
            setSpellRecovery(pCastSpell, recoveryTime);
            pAudioPlayer->playSpellSound(pCastSpell->uSpellID, PID_INVALID);
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
                                CombinedSkillValue skill_level,
                                SpellCastFlags uFlags,
                                int spell_sound_id) {
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
                             CombinedSkillValue skill_value,
                             SpellCastFlags flags,
                             int a6) {
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage == TE_WAIT ||
            pTurnEngine->turn_stage == TE_MOVEMENT) {
            return;
        }
    }

    CombinedSkillValue checkSkill = skill_value;
    assert(uPlayerID < 4);
    Character *character = &pParty->pCharacters[uPlayerID];
    if (!(flags & ON_CAST_TargetIsParty)) {
        switch (spell) {
            case SPELL_SPIRIT_FATE:
            case SPELL_BODY_FIRST_AID:
            case SPELL_DARK_REANIMATE:
                flags |= ON_CAST_TargetedActorOrCharacter;
                break;

            case SPELL_FIRE_FIRE_AURA:
            case SPELL_WATER_RECHARGE_ITEM:
            case SPELL_WATER_ENCHANT_ITEM:
            case SPELL_DARK_VAMPIRIC_WEAPON:
                flags |= ON_CAST_TargetedEnchantment;
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
                    flags |= ON_CAST_TargetedActor;
                }
                break;
            case SPELL_MIND_TELEPATHY:
            case SPELL_MIND_BERSERK:
            case SPELL_MIND_ENSLAVE:
            case SPELL_LIGHT_PARALYZE:
            case SPELL_DARK_CONTROL_UNDEAD:
                flags |= ON_CAST_TargetedActor;
                break;

            case SPELL_EARTH_TELEKINESIS:
                flags |= ON_CAST_TargetedTelekinesis;
                break;

            case SPELL_SPIRIT_BLESS:
                if (!checkSkill) {
                    checkSkill = character->pActiveSkills[CHARACTER_SKILL_SPIRIT];
                }
                if (checkSkill.mastery() < CHARACTER_SKILL_MASTERY_EXPERT && !engine->config->debug.AllMagic.value()) {
                    flags |= ON_CAST_TargetedCharacter;
                }
                break;

            case SPELL_SPIRIT_PRESERVATION:
                if (!checkSkill) {
                    checkSkill = character->pActiveSkills[CHARACTER_SKILL_SPIRIT];
                }
                if (checkSkill.mastery() < CHARACTER_SKILL_MASTERY_MASTER && !engine->config->debug.AllMagic.value()) {
                    flags |= ON_CAST_TargetedCharacter;
                }
                break;

            case SPELL_DARK_PAIN_REFLECTION:
                if (!checkSkill) {
                    checkSkill = character->pActiveSkills[CHARACTER_SKILL_DARK];
                }
                if (checkSkill.mastery() < CHARACTER_SKILL_MASTERY_MASTER && !engine->config->debug.AllMagic.value()) {
                    flags |= ON_CAST_TargetedCharacter;
                }
                break;

            case SPELL_BODY_HAMMERHANDS:
                if (!checkSkill) {
                    checkSkill = character->pActiveSkills[CHARACTER_SKILL_BODY];
                }
                if (checkSkill.mastery() < CHARACTER_SKILL_MASTERY_GRANDMASTER && !engine->config->debug.AllMagic.value()) {
                    flags |= ON_CAST_TargetedCharacter;
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
                flags |= ON_CAST_TargetedCharacter;
                break;

            case SPELL_DARK_SACRIFICE:
                flags |= ON_CAST_TargetedHireling;
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
    if (result != -1 && pGUIWindow_CastTargetedSpell == nullptr) {
        Sizei renDims = render->GetRenderDimensions();
        if (flags & ON_CAST_TargetedCharacter) {
            pGUIWindow_CastTargetedSpell = new TargetedSpellUI_Character({0, 0}, renDims, &pCastSpellInfo[result]);
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
        if (flags & ON_CAST_TargetedActor) {
            pGUIWindow_CastTargetedSpell = new TargetedSpellUI_Actor({0, 0}, renDims, &pCastSpellInfo[result]);
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
        if (flags & ON_CAST_TargetedTelekinesis) {
            pGUIWindow_CastTargetedSpell = new TargetedSpellUI_Telekinesis({0, 0}, renDims, &pCastSpellInfo[result]);
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
        if (flags & ON_CAST_TargetedEnchantment) {
            pGUIWindow_CastTargetedSpell = pCastSpellInfo[result].GetCastSpellInInventoryWindow();
            IsEnchantingInProgress = true;
            enchantingActiveCharacter = pParty->activeCharacterIndex();
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
        if (flags & ON_CAST_TargetedActorOrCharacter) {
            pGUIWindow_CastTargetedSpell = new TargetedSpellUI_ActorOrCharacter({0, 0}, renDims, &pCastSpellInfo[result]);
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
        if (flags & ON_CAST_TargetedHireling) {
            pGUIWindow_CastTargetedSpell = new TargetedSpellUI_Hirelings({0, 0}, renDims, &pCastSpellInfo[result]);
            // Next line was added to do something with picked item on Sacrifice cast
            pParty->placeHeldItemInInventoryOrDrop();
            return;
        }
    }
}

void pushTempleSpell(SPELL_TYPE spell) {
    CombinedSkillValue skill_value = CombinedSkillValue(pParty->uCurrentDayOfMonth % 7 + 1, CHARACTER_SKILL_MASTERY_MASTER);

    pushSpellOrRangedAttack(spell, pParty->activeCharacterIndex() - 1, skill_value,
                            ON_CAST_TargetIsParty | ON_CAST_NoRecoverySpell, 0);
}

void pushNPCSpell(SPELL_TYPE spell) {
    pushSpellOrRangedAttack(spell, 0, SCROLL_OR_NPC_SPELL_SKILL_VALUE, 0, 0);
}

void pushScrollSpell(SPELL_TYPE spell, unsigned int uPlayerID) {
    pushSpellOrRangedAttack(spell, uPlayerID, SCROLL_OR_NPC_SPELL_SKILL_VALUE, ON_CAST_CastViaScroll, 0);
}

void spellTargetPicked(int pid, int playerTarget) {
    assert((pid != PID_INVALID) ^ (playerTarget != -1));

    CastSpellInfo *pCastSpell = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
    pCastSpell->uFlags &= ~ON_CAST_CastingInProgress;
    pCastSpell->spell_target_pid = pid;
    // TODO(Nik-RE-dev): need to get rid of uPlayerID_2 and use pid with OBJECT_Character and something else for hirelings.
    pCastSpell->uPlayerID_2 = playerTarget;
    // TODO(Nik-RE-dev): why recovery time is set here?
    pParty->pCharacters[pCastSpell->uPlayerID].SetRecoveryTime(300);
}
