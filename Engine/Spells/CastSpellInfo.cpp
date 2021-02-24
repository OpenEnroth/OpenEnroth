#include "Engine/Spells/CastSpellInfo.h"

#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/Events.h"
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

#include "Platform/OSWindow.h"


using EngineIoc = Engine_::IocContainer;

static SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

const size_t CastSpellInfoCount = 10;
std::array<CastSpellInfo, CastSpellInfoCount> pCastSpellInfo;

//----- (00427E01) --------------------------------------------------------
void CastSpellInfoHelpers::_427E01_cast_spell() {
    CastSpellInfo *pCastSpell;        // ebx@2
    signed int spell_pointed_target;  // eax@14

    int monster_id;        // ecx@184
    int spell_overlay_id;  // eax@274
    int dist_X;            // eax@278
    int dist_Z;
    int spell_spray_angle_start;     // esi@369
    int spell_spray_angle_end;       // edi@369
    double spell_recharge_factor;    // st7@478
    ItemGen *spell_item_to_enchant;  // edi@492


    // int *v267;              // eax@524
    // int v268;              // eax@524
    // int v278;              // ecx@548
    // char v279;             // al@550
    // int v280;              // eax@552
    // int *v281;             // esi@553
    // int v282;              // edx@555
    // int *v283;             // ecx@555
    // int v284;              // esi@555
    // int v285;              // edx@555
    // int *l;                // eax@556
    // int v295;              // edx@575

    const char *v317;      // ecx@617
                           //  int v396; // eax@752
    __int16 v448;          // ax@864
    DDM_DLV_Header *v613;  // eax@1108
    int v642;              // edi@1156
    int v643;              // eax@1156
    int v659;              // [sp+0h] [bp-E84h]@123
    // unsigned __int64 v663; // [sp+0h] [bp-E84h]@639
    int target_undead;              // [sp+4h] [bp-E80h]@12
    PLAYER_SKILL_TYPE which_skill;  // [sp+4h] [bp-E80h]@25

    unsigned __int64 v685;          // [sp+D08h] [bp-17Ch]@416
    unsigned __int64 v687;          // [sp+D24h] [bp-160h]@327
    Vec3_int_ v688;                 // [sp+D2Ch] [bp-158h]@943
    Vec3_int_ v691;                 // [sp+D38h] [bp-14Ch]@137
    Vec3_int_ v694;                 // [sp+D44h] [bp-140h]@982
    Vec3_int_ v697;                 // [sp+D50h] [bp-134h]@129
    Vec3_int_ v700;                 // [sp+D5Ch] [bp-128h]@339
    Vec3_int_ v701;                 // [sp+D68h] [bp-11Ch]@286
    Vec3_int_ v704;                 // [sp+D74h] [bp-110h]@132
    Vec3_int_ v707;                 // [sp+D80h] [bp-104h]@1127
    // int v710;                       // [sp+D8Ch] [bp-F8h]@1156

    AIDirection target_direction;  // [sp+DA4h] [bp-E0h]@21

    int dist_Y;           // [sp+DD8h] [bp-ACh]@163
    int spell_level = 0;  // [sp+E4Ch] [bp-38h]@1
    ItemGen *_this = 0;   // [sp+E50h] [bp-34h]@23
             // [sp+E54h] [bp-30h]@23
    int buff_resist;
    bool spell_sound_flag = false;  // [sp+E5Ch] [bp-28h]@1
    Player *pPlayer;                // [sp+E64h] [bp-20h]@8
    // char *v730;                       // [sp+E68h] [bp-1Ch]@53
    int v730_int;
    ItemGen *v730c;
    int skill_level = 0;  // [sp+E6Ch] [bp-18h]@48
    // int *v732;      // [sp+E70h] [bp-14h]@325
    int v732_int;

    int spellduration;
    signed int spell_targeted_at = 0;  // [sp+E7Ch] [bp-8h]@14
    int amount = 0;                    // [sp+E80h] [bp-4h]@1
    int obj_type;
    ItemDesc *_item;

    SpriteObject pSpellSprite;  // [sp+DDCh] [bp-A8h]@1

    for (int n = 0; n < CastSpellInfoCount; ++n) {  // cycle through spell queue
        pCastSpell = &pCastSpellInfo[n];
        if (pCastSpell->uSpellID == 0)
            continue;  // spell item blank skip to next

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

        spell_targeted_at = pCastSpell->spell_target_pid;

        if (spell_targeted_at == 0) {  // no target ?? test
            if (pCastSpell->uSpellID == SPELL_LIGHT_DESTROY_UNDEAD ||
                pCastSpell->uSpellID == SPELL_SPIRIT_TURN_UNDEAD ||
                pCastSpell->uSpellID == SPELL_DARK_CONTROL_UNDEAD)
                target_undead = 1;
            else
                target_undead = 0;

            // find the closest target
            spell_targeted_at = stru_50C198.FindClosestActor(5120, 1, target_undead);
            spell_pointed_target = mouse->uPointingObjectID;

            if (mouse->uPointingObjectID &&
                PID_TYPE(spell_pointed_target) == OBJECT_Actor &&
                pActors[PID_ID(spell_pointed_target)].CanAct())  // check can act
                spell_targeted_at = mouse->uPointingObjectID;
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
                target_direction.uPitchAngle = -pParty->sRotationX;
            }
        }

        if (pCastSpell->forced_spell_skill_level) {
            // for spell scrolls - decode spell power and mastery
            spell_level = (pCastSpell->forced_spell_skill_level) & 0x3F;  // 6 bytes
            skill_level = ((pCastSpell->forced_spell_skill_level) & 0x1C0) / 64 + 1;
        } else {
            if (pCastSpell->uSpellID < SPELL_AIR_WIZARD_EYE)
                which_skill = PLAYER_SKILL_FIRE;
            else if (pCastSpell->uSpellID < SPELL_WATER_AWAKEN)
                which_skill = PLAYER_SKILL_AIR;
            else if (pCastSpell->uSpellID < SPELL_EARTH_STUN)
                which_skill = PLAYER_SKILL_WATER;
            else if (pCastSpell->uSpellID < SPELL_SPIRIT_DETECT_LIFE)
                which_skill = PLAYER_SKILL_EARTH;
            else if (pCastSpell->uSpellID < SPELL_MIND_REMOVE_FEAR)
                which_skill = PLAYER_SKILL_SPIRIT;
            else if (pCastSpell->uSpellID < SPELL_BODY_CURE_WEAKNESS)
                which_skill = PLAYER_SKILL_MIND;
            else if (pCastSpell->uSpellID < SPELL_LIGHT_LIGHT_BOLT)
                which_skill = PLAYER_SKILL_BODY;
            else if (pCastSpell->uSpellID < SPELL_DARK_REANIMATE)
                which_skill = PLAYER_SKILL_LIGHT;
            else if (pCastSpell->uSpellID < SPELL_BOW_ARROW)
                which_skill = PLAYER_SKILL_DARK;
            else if (pCastSpell->uSpellID == SPELL_BOW_ARROW)
                which_skill = PLAYER_SKILL_BOW;
            else if (pCastSpell->uSpellID == SPELL_101 ||
                pCastSpell->uSpellID == SPELL_LASER_PROJECTILE)
                which_skill = PLAYER_SKILL_BLASTER;
            else
                assert(false && "Unknown spell");

            spell_level = pPlayer->GetActualSkillLevel(which_skill);
            skill_level = pPlayer->GetActualSkillMastery(which_skill);

            if (engine->config->debug_all_magic) {
                spell_level = 10;
                skill_level = 4;
            }
        }

        if (pCastSpell->uSpellID < SPELL_BOW_ARROW) {
            if (pCastSpell->forced_spell_skill_level || engine->config->debug_all_magic)
                uRequiredMana = 0;
            else
                uRequiredMana = pSpellDatas[pCastSpell->uSpellID].mana_per_skill[skill_level - 1];

            sRecoveryTime = pSpellDatas[pCastSpell->uSpellID].recovery_per_skill[skill_level - 1];
        }

        if (!pCastSpell->forced_spell_skill_level) {
            if (which_skill == PLAYER_SKILL_DARK && pParty->uCurrentHour == 0 &&
                pParty->uCurrentMinute == 0 ||
                which_skill == PLAYER_SKILL_LIGHT && pParty->uCurrentHour == 12 &&
                pParty->uCurrentMinute == 0)  // free spells at midnight or midday
                uRequiredMana = 0;
        }

        if (pCastSpell->uSpellID < SPELL_BOW_ARROW &&
            pPlayer->sMana < uRequiredMana) {
            GameUI_StatusBar_OnEvent(
                localization->GetString(586));  // "Not enough spell points"
            pCastSpell->uSpellID = 0;
            continue;
        }

        if (pPlayer->IsCursed() && pCastSpell->uSpellID < SPELL_BOW_ARROW &&
            rand() % 100 < 50) {  //неудачное кастование - player is cursed
            if (!pParty->bTurnBasedModeOn) {
                pPlayer->SetRecoveryTime(
                    (int64_t)(flt_6BE3A4_debug_recmod1 * 213.3333333333333));
            } else {
                pParty->pTurnBasedPlayerRecoveryTimes[pCastSpellInfo[n].uPlayerID] = 100;
                pPlayer->SetRecoveryTime(sRecoveryTime);
                pTurnEngine->ApplyPlayerAction();
            }

            GameUI_StatusBar_OnEvent(
                localization->GetString(428));  // "Spell failed"
            pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
            pCastSpellInfo[n].uSpellID = 0;
            pPlayer->sMana -= uRequiredMana;
            return;
        }

        // do mana check here?

        switch (pCastSpell->uSpellID) {
            case SPELL_101:
                assert(false && "Unknown spell effect #101 (prolly flaming bow arrow");

            case SPELL_BOW_ARROW:  //стрельба из лука
            {
                amount = 1;
                if (skill_level >= 3) amount = 2;

                sRecoveryTime = pPlayer->GetAttackRecoveryTime(true);
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                if (pPlayer->WearsItem(ITEM_ARTEFACT_ULLYSES, EQUIP_BOW))
                    pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(0xBD6u);
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.vPosition.z = pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.spell_caster_pid = PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID];
                memcpy(
                    &pSpellSprite.containing_item,
                    &pPlayer->pInventoryItemList[pPlayer->pEquipment.uBow - 1],
                    sizeof(pSpellSprite.containing_item));
                pSpellSprite.uAttributes = 256;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes = 260;
                }
                for (int i = 0; i < amount; ++i) {
                    if (i)
                        pSpellSprite.vPosition.z += 32;
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pSpellSprite.vPosition.x, pSpellSprite.vPosition.y,
                        pSpellSprite.vPosition.z);
                    if (pSpellSprite.Create(target_direction.uYawAngle,
                        target_direction.uPitchAngle,
                        pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                        pCastSpell->uPlayerID + 1) != -1 &&
                        pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                }
                break;
            }

            case SPELL_LASER_PROJECTILE:  //стрельба из бластера
            {
                sRecoveryTime = pPlayer->GetAttackRecoveryTime(0);
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.vPosition.z = pParty->vPosition.z + (signed int)pParty->uPartyHeight / 2;
                pSpellSprite.spell_caster_pid = PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID];
                memcpy(
                    &pSpellSprite.containing_item,
                    &pPlayer->pInventoryItemList[pPlayer->pEquipment.uMainHand -
                                                 1],
                    sizeof(pSpellSprite.containing_item));
                // &pParty->pPlayers[pCastSpell->uPlayerID].spellbook.pDarkSpellbook.bIsSpellAvailable[36
                // *
                // pParty->pPlayers[pCastSpell->uPlayerID].pEquipment.uMainHand +
                // 5], );
                pSpellSprite.uSectorID =
                    pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                       pSpellSprite.vPosition.z);
                pSpellSprite.uAttributes |= 0x100;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                if (pSpellSprite.Create(
                    target_direction.uYawAngle,
                    target_direction.uPitchAngle,
                    pObjectList->pObjects[pSpellSprite.uObjectDescID]
                    .uSpeed,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                break;
            }

            case SPELL_FIRE_TORCH_LIGHT:  //свет факела
            {
                switch (skill_level) {
                    case 1:
                        amount = 2;
                        break;
                    case 2:
                        amount = 3;
                        break;
                    case 3:
                    case 4:
                        amount = 4;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;

                pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        (GameTime::FromSeconds(3600 * spell_level))),
                    skill_level, amount, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_FIRE_SPIKE:  //огненный шип
            {
                switch (skill_level) {
                    case 1:
                        amount = 3;
                        break;
                    case 2:
                        amount = 5;
                        break;
                    case 3:
                        amount = 7;
                        break;
                    case 4:
                        amount = 9;
                        break;
                    default:
                        assert(false);
                }
                int _v733 = 0;
                for (uint i = 0; i < uNumSpriteObjects; ++i) {
                    SpriteObject *object = &pSpriteObjects[i];
                    if (object->uType &&
                        object->spell_id == SPELL_FIRE_FIRE_SPIKE &&
                        object->spell_caster_pid ==
                            PID(OBJECT_Player, pCastSpell->uPlayerID))
                        ++_v733;
                }
                if (_v733 > amount) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pParty->vPosition.x, pParty->vPosition.y,
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = (short)target_direction.uYawAngle;
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(pParty->sRotationZ,
                    pParty->sRotationX + 10, v659,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_IMPLOSION:  //Точный взрыв
            {
                monster_id = PID_ID(spell_targeted_at);
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (!spell_targeted_at) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    v697.x = 0;
                    v697.y = 0;
                    v697.z = 0;
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.spell_target_pid =
                        PID(OBJECT_Actor, monster_id);
                    Actor::DamageMonsterFromParty(
                        PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                        monster_id, &v697);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_EARTH_MASS_DISTORTION:  //Изменение веса
            {
                monster_id = PID_ID(spell_targeted_at);
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (pActors[monster_id].DoesDmgTypeDoDamage((DAMAGE_TYPE)3)) {
                    pActors[monster_id]
                        .pActorBuffs[ACTOR_BUFF_MASS_DISTORTION]
                        .Apply(GameTime(pMiscTimer->uTotalGameTimeElapsed + 128), 0, 0, 0,
                               0);
                    v704.x = 0;
                    v704.y = 0;
                    v704.z = 0;
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.field_60_distance_related_prolly_lod = 0;
                    pSpellSprite.uFacing = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z;
                    pSpellSprite.spell_target_pid =
                        PID(OBJECT_Actor, monster_id);
                    Actor::DamageMonsterFromParty(
                        PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                        monster_id, &v704);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_DESTROY_UNDEAD:  //Уничтожение нежити
            {
                if (!pPlayer->CanCastSpell(uRequiredMana) ||
                    !spell_targeted_at ||
                    PID_TYPE(spell_targeted_at) != OBJECT_Actor)
                    break;
                // v730 = spell_targeted_at >> 3;
                // HIDWORD(spellduration) =
                // (int)&pActors[PID_ID(spell_targeted_at)];
                v691.x = 0;
                v691.y = 0;
                v691.z = 0;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.x =
                    pActors[PID_ID(spell_targeted_at)].vPosition.x;
                pSpellSprite.vPosition.y =
                    pActors[PID_ID(spell_targeted_at)].vPosition.y;
                pSpellSprite.vPosition.z =
                    pActors[PID_ID(spell_targeted_at)].vPosition.z;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pSpellSprite.vPosition.x, pSpellSprite.vPosition.y,
                    pSpellSprite.vPosition.z);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uAttributes |= 0x80u;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                int obj_id = pSpellSprite.Create(0, 0, 0, 0);
                if (!MonsterStats::BelongsToSupertype(
                        pActors[PID_ID(spell_targeted_at)].pMonsterInfo.uID,
                        MONSTER_SUPERTYPE_UNDEAD)) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                Actor::DamageMonsterFromParty(PID(OBJECT_Item, obj_id),
                                              PID_ID(spell_targeted_at), &v691);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_FIRE_BOLT:      //Удар огня
            case SPELL_FIRE_FIREBALL:       //Огненный шар
            case SPELL_FIRE_INCINERATE:     //Испепеление
            case SPELL_AIR_LIGHNING_BOLT:   //Удар молнии
            case SPELL_WATER_ICE_BOLT:      //Льдяная молния
            case SPELL_WATER_ICE_BLAST:     //Льдяной взрыв
            case SPELL_EARTH_STUN:          //Оглушить
            case SPELL_EARTH_DEADLY_SWARM:  //Рой смерти
            case SPELL_MIND_MIND_BLAST:     //Удар мысли
            case SPELL_MIND_PSYCHIC_SHOCK:  //Психический шок
            case SPELL_BODY_HARM:           //Вред
            case SPELL_LIGHT_LIGHT_BOLT:    //Луч света
            case SPELL_DARK_DRAGON_BREATH:  //Дыхание дракона
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.uAttributes = 0;
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pParty->vPosition.x, pParty->vPosition.y,
                        pParty->vPosition.z +
                            (signed int)pParty->uPartyHeight / 3);
                else
                    pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                if (pCastSpell->uSpellID == SPELL_AIR_LIGHNING_BOLT)
                    pSpellSprite.uAttributes |= 0x40;
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(target_direction.uYawAngle,
                    target_direction.uPitchAngle, v659,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_ACID_BURST:  //Всплеск кислоты
            case SPELL_EARTH_BLADES:      //Лезвия
            case SPELL_BODY_FLYING_FIST:  //Летающий кулак
            case SPELL_DARK_TOXIC_CLOUD:  //Облако-токсин
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 2;
                pSpellSprite.uSectorID =
                    pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                       pSpellSprite.vPosition.z);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(target_direction.uYawAngle,
                    target_direction.uPitchAngle, v659,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_SUNRAY: {  //Луч Солнца
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor ||
                    uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
                        (pParty->uCurrentHour < 5 ||
                         pParty->uCurrentHour >= 21)) {  //только в дневное время
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (pPlayer->CanCastSpell(uRequiredMana)) {
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.vPosition.x = pParty->vPosition.x;
                    pSpellSprite.vPosition.y = pParty->vPosition.y;
                    pSpellSprite.vPosition.z =
                        pParty->vPosition.z +
                        (signed int)pParty->uPartyHeight / 3;
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pParty->vPosition.x, pParty->vPosition.y,
                        pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= 4;
                    }
                    v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID]
                               .uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle,
                                            target_direction.uPitchAngle, v659,
                                            pCastSpell->uPlayerID + 1) != -1 &&
                        pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    spell_sound_flag = true;
                }
                break;
            }

            case SPELL_LIGHT_PARALYZE:  //Паралич
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                monster_id = PID_ID(spell_targeted_at);
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor &&
                    pActors[monster_id].DoesDmgTypeDoDamage((DAMAGE_TYPE)9)) {
                    Actor::AI_Stand(PID_ID(spell_targeted_at), 4, 0x80, 0);
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_PARALYZED].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(3 * 60 * spell_level)),
                        skill_level, 0, 0, 0);
                    pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                    pActors[monster_id].vVelocity.x = 0;
                    pActors[monster_id].vVelocity.y = 0;
                    spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(&pActors[monster_id], 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_EARTH_SLOW:  //Замедление
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 180 * spell_level;
                        amount = 2;
                        break;  // LODWORD(spellduration)???не применяется далее
                    case 2:
                        spellduration = 300 * spell_level;
                        amount = 2;
                        break;
                    case 3:
                        spellduration = 300 * spell_level;
                        amount = 4;
                        break;
                    case 4:
                        spellduration = 300 * spell_level;
                        amount = 8;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                // v721 = 836 * PID_ID(spell_targeted_at);
                monster_id = PID_ID(spell_targeted_at);
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor &&
                    pActors[monster_id].DoesDmgTypeDoDamage((DAMAGE_TYPE)3)) {
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_SLOWED].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(spellduration)),
                        skill_level, amount, 0, 0);
                    pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                    spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(&pActors[monster_id], 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_CHARM:  // Очарование
            {
                monster_id = PID_ID(spell_targeted_at);
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (pActors[monster_id].DoesDmgTypeDoDamage((DAMAGE_TYPE)7)) {
                    uint power = 300 * spell_level;
                    if (skill_level == 2)
                        power = 600 * spell_level;
                    else if (skill_level == 3)
                        power = 29030400;

                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                    pActors[monster_id]
                        .pActorBuffs[ACTOR_BUFF_ENSLAVED]
                        .Reset();
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(power << 7)),
                        skill_level, 0, 0, 0);
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z +
                                               pActors[monster_id].uActorHeight;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID =
                        pIndoor->GetSector(pActors[monster_id].vPosition.x,
                                           pActors[monster_id].vPosition.y,
                                           pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= 0x80u;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_SHRINKING_RAY:  //Сжимающий луч
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pParty->vPosition.x, pParty->vPosition.y,
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = target_direction.uYawAngle;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.spell_id = SPELL_FIRE_PROTECTION_FROM_FIRE;
                pSpellSprite.spell_level = 300 * spell_level;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(target_direction.uYawAngle,
                    target_direction.uPitchAngle, v659,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_FIRE_AURA:  //Аура огня
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 3600 * spell_level;
                        amount = 10;
                        break;  //Огня
                    case 2:
                        spellduration = 3600 * spell_level;
                        amount = 11;
                        break;  //Огненного жара
                    case 3:
                        spellduration = 3600 * spell_level;
                        amount = 12;
                        break;
                    case 4:
                        spellduration = 0;
                        amount = 12;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                v730c = &pParty->pPlayers[pCastSpell->uPlayerID_2]
                             .pInventoryItemList[spell_targeted_at - 1];
                _item = &pItemsTable->pItems[v730c->uItemID];
                v730c->UpdateTempBonus(pParty->GetPlayingTime());
                if (v730c->uItemID < 64 ||
                    v730c->uItemID > 65 && !v730c->IsBroken() &&
                        !v730c->special_enchantment &&
                        !v730c->uEnchantmentType &&
                        (_item->uEquipType == EQUIP_SINGLE_HANDED ||
                         _item->uEquipType == EQUIP_TWO_HANDED ||
                         _item->uEquipType == EQUIP_BOW) &&
                        !pItemsTable->IsMaterialNonCommon(v730c)) {
                    v730c->special_enchantment = (ITEM_ENCHANTMENT)amount;
                    if (skill_level != 4) {
                        v730c->uExpireTime =
                            GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(spellduration));
                        v730c->uAttributes |= ITEM_TEMP_BONUS;
                    }
                    v730c->uAttributes |= ITEM_AURA_EFFECT_RED;
                    _50C9A8_item_enchantment_timer = 256;
                    spell_sound_flag = true;
                    break;
                }
                _50C9D0_AfterEnchClickEventId = 113;
                _50C9D4_AfterEnchClickEventSecondParam = 0;
                _50C9D8_AfterEnchClickEventTimeout = 1;
                GameUI_StatusBar_OnEvent(
                    localization->GetString(428));  // Spell failed
                pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0,
                                        0);
                pCastSpell->uSpellID = 0;
                continue;
            }

            case SPELL_BODY_REGENERATION:  //Регенерация
            {
                switch (skill_level) {
                    case 1:
                        amount = 1;
                        break;
                    case 2:
                        amount = 1;
                        break;
                    case 3:
                        amount = 3;
                        break;
                    case 4:
                        amount = 10;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana))
                    break;
                spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                pParty->pPlayers[pCastSpell->uPlayerID_2]
                    .pPlayerBuffs[PLAYER_BUFF_REGENERATION]
                    .Apply(GameTime(pParty->GetPlayingTime() +
                               GameTime::FromSeconds(3600 * spell_level)),
                           skill_level, amount, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_PROTECTION_FROM_FIRE:  //Защита от Огня
            case SPELL_AIR_PROTECTION_FROM_AIR:  //Звщита от Воздуха
            case SPELL_WATER_PROTECTION_FROM_WATER:  //Защита от Воды
            case SPELL_EARTH_PROTECTION_FROM_EARTH:  //Защита от Земли
            case SPELL_MIND_PROTECTION_FROM_MIND:  //Защита от Мысли
            case SPELL_BODY_PROTECTION_FROM_BODY:  //Защита от Тела
            {
                switch (skill_level) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        amount = skill_level * spell_level;
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
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                        GameTime::FromSeconds(3600 * spell_level)),
                    skill_level, amount, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_HASTE:  //Спешка
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 60 * (spell_level + 60);
                        break;
                    case 2:
                        spellduration = 60 * (spell_level + 60);
                        break;
                    case 3:
                        spellduration = 180 * (spell_level + 20);
                        break;
                    case 4:
                        spellduration = 240 * (spell_level + 15);
                        break;
                    default:
                        assert(false);
                }
                if (pPlayer->CanCastSpell(uRequiredMana)) {
                    spell_sound_flag = true;
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        if (pParty->pPlayers[pl_id]
                                .conditions_times[Condition_Weak]
                                .Valid())
                            spell_sound_flag = false;
                    }
                    if (spell_sound_flag) {
                        pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(
                            GameTime(pParty->GetPlayingTime() +
                                GameTime::FromSeconds(spellduration)),
                            skill_level, 0, 0, 0);
                        spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 0);
                        spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 1);
                        spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 2);
                        spell_fx_renderer->SetPlayerBuffAnim(
                            pCastSpell->uSpellID, 3);
                    }
                }
                break;
            }

            case SPELL_SPIRIT_BLESS:  //Благословение
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 2:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 3:
                        spellduration = 900 * (spell_level + 4);
                        break;
                    case 4:
                        spellduration = 3600 * (spell_level + 1);
                        break;
                    default:
                        assert(false);
                }
                amount = spell_level + 5;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (skill_level == 1) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    spell_overlay_id = pOtherOverlayList->_4418B1(
                        10000, pCastSpell->uPlayerID_2 + 310, 0, 65536);
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .pPlayerBuffs[PLAYER_BUFF_BLESS]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(spellduration)),
                               1, amount, spell_overlay_id, 0);
                    spell_sound_flag = true;
                    break;
                }
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_id);
                    spell_overlay_id = pOtherOverlayList->_4418B1(
                        10000, pl_id + 310, 0, 65536);
                    pParty->pPlayers[pl_id].pPlayerBuffs[1].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(spellduration)),
                        skill_level, amount, spell_overlay_id, 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_SPIRIT_LASH:  //Плеть Духа
            {
                if (pPlayer->CanCastSpell(uRequiredMana) && spell_targeted_at &&
                    PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    int monster_id = PID_ID(spell_targeted_at);
                    dist_X = abs(pActors[monster_id].vPosition.x -
                                 pParty->vPosition.x);
                    dist_Y = abs(pActors[monster_id].vPosition.y -
                                 pParty->vPosition.y);
                    dist_Z = abs(pActors[monster_id].vPosition.z -
                                 pParty->vPosition.z);
                    int count = int_get_vector_length(dist_X, dist_Y, dist_Z);
                    if ((double)count <= 307.2) {
                        v701.x = 0;
                        v701.y = 0;
                        v701.z = 0;
                        pSpellSprite.containing_item.Reset();
                        pSpellSprite.spell_id = pCastSpell->uSpellID;
                        pSpellSprite.spell_level = spell_level;
                        pSpellSprite.spell_skill = skill_level;
                        pSpellSprite.uObjectDescID =
                            pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                        pSpellSprite.uAttributes = 0;
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.uSpriteFrameID = 0;
                        pSpellSprite.field_60_distance_related_prolly_lod = 0;
                        pSpellSprite.spell_caster_pid =
                            PID(OBJECT_Player, pCastSpell->uPlayerID);
                        pSpellSprite.uFacing = 0;
                        pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                        pSpellSprite.vPosition.x =
                            pActors[monster_id].vPosition.x;
                        pSpellSprite.vPosition.y =
                            pActors[monster_id].vPosition.y;
                        pSpellSprite.vPosition.z =
                            pActors[monster_id].vPosition.z -
                            (int)((double)pActors[monster_id].uActorHeight *
                                  -0.8);
                        pSpellSprite.spell_target_pid =
                            PID(OBJECT_Actor, spell_targeted_at);
                        Actor::DamageMonsterFromParty(
                            PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                            monster_id, &v701);
                        spell_sound_flag = true;
                    } else {
                        GameUI_StatusBar_OnEvent(localization->GetString(
                            428));  // Spell failed     Не получилось!!!
                        pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                        pCastSpell->uSpellID = 0;
                    }
                }
                break;
            }

            case SPELL_AIR_SHIELD:       //Щит
            case SPELL_EARTH_STONESKIN:  //Каменная кожа
            case SPELL_SPIRIT_HEROISM:   //Героизм
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 2:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 3:
                        spellduration = 900 * (spell_level + 4);
                        break;
                    case 4:
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
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                    skill_level, amount, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_IMMOLATION:  //Кольцо огня
            {
                if (skill_level == 4)
                    spellduration = 600 * spell_level;
                else
                    spellduration = 60 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                    skill_level, spell_level, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_METEOR_SHOWER:  //Поток метеоров
            {
                // if (skill_level < 3)//для мастера и магистра
                // break;
                int meteor_num;
                if (skill_level == 4)
                    meteor_num = 20;
                else
                    meteor_num = 16;
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        491));  // Can't cast Meteor Shower indoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana))
                    break;
                obj_type = PID_TYPE(spell_targeted_at);
                monster_id = PID_ID(spell_targeted_at);
                if (obj_type ==
                    OBJECT_Actor) {  // если закл поставить на быстрое закл., то
                                     // можно указывать куда кидать метеоры
                    dist_X = pActors[monster_id].vPosition.x;
                    dist_Y = pActors[monster_id].vPosition.y;
                    dist_Z = pActors[monster_id].vPosition.z;
                } else {
                    dist_X = pParty->vPosition.x + 2048 * pIndoorCameraD3D->fRotationZCosine;
                    dist_Y = pParty->vPosition.y + 2048 * pIndoorCameraD3D->fRotationZSine;
                    dist_Z = pParty->vPosition.z;
                }
                unsigned __int64 k = 0;
                int j = 0;
                if (meteor_num > 0) {
                    v730_int = obj_type == OBJECT_Actor ? spell_targeted_at : 0;
                    for (meteor_num; meteor_num; meteor_num--) {
                        spell_targeted_at = rand() % 1000;
                        if (sqrt(((double)spell_targeted_at - 2500) *
                                     ((double)spell_targeted_at - 2500) +
                                 j * j + k * k) <= 1.0) {
                            HEXRAYS_LODWORD(v687) = 0;
                            HEXRAYS_HIDWORD(v687) = 0;
                        } else {
                            HEXRAYS_HIDWORD(v687) = TrigLUT->Atan2(
                                (signed __int64)sqrt((float)(j * j + k * k)),
                                (double)spell_targeted_at - 2500);
                            HEXRAYS_LODWORD(v687) = TrigLUT->Atan2(j, k);
                        }
                        pSpellSprite.containing_item.Reset();
                        pSpellSprite.spell_id = pCastSpell->uSpellID;
                        pSpellSprite.spell_level = spell_level;
                        pSpellSprite.spell_skill = skill_level;
                        pSpellSprite.uObjectDescID =
                            pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                        pSpellSprite.uAttributes = 0;
                        pSpellSprite.vPosition.x = dist_X;
                        pSpellSprite.vPosition.y = dist_Y;
                        pSpellSprite.vPosition.z = spell_targeted_at + dist_Z;
                        pSpellSprite.uSectorID = 0;
                        pSpellSprite.uSpriteFrameID = 0;
                        pSpellSprite.spell_caster_pid =
                            PID(OBJECT_Player, pCastSpell->uPlayerID);
                        pSpellSprite.spell_target_pid = v730_int;
                        pSpellSprite.field_60_distance_related_prolly_lod =
                            stru_50C198._427546(spell_targeted_at + 2500);
                        pSpellSprite.uFacing = v687;
                        pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                        if (pParty->bTurnBasedModeOn) {
                            pSpellSprite.uAttributes = 4;
                        }
                        if (pSpellSprite.Create(v687, HEXRAYS_SHIDWORD(v687),
                                                pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed, 0) != -1 &&
                                                pParty->bTurnBasedModeOn) {
                            ++pTurnEngine->pending_actions;
                        }
                        j = rand() % 1024 - 512;
                        k = rand() % 1024 - 512;
                    }
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_FIRE_INFERNO:  //Адский огонь
            {
                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        492));  // Can't cast Inferno outdoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana))
                    break;
                int mon_num = render->GetActorsInViewport(4096);
                v700.x = 0;
                v700.y = 0;
                v700.z = 0;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                for (uint i = 0; i < mon_num; i++) {
                    pSpellSprite.vPosition.x =
                        pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.x;
                    pSpellSprite.vPosition.y =
                        pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.y;
                    pSpellSprite.vPosition.z =
                        pActors[_50BF30_actors_in_viewport_ids[i]].vPosition.z -
                        (unsigned int)(signed __int64)((double)pActors
                                                           [_50BF30_actors_in_viewport_ids
                                                                [i]]
                                                               .uActorHeight *
                                                       -0.8);
                    pSpellSprite.spell_target_pid =
                        PID(OBJECT_Actor, _50BF30_actors_in_viewport_ids[i]);
                    Actor::DamageMonsterFromParty(
                        PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                        _50BF30_actors_in_viewport_ids[i], &v700);
                    spell_fx_renderer->RenderAsSprite(
                        &pSpellSprite);
                    spell_fx_renderer
                        ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(
                            0xFF3C1E, 0x40);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_WIZARD_EYE:  //Око чародея
            {
                spellduration = 3600 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (uint pl_id = 0; pl_id < 4; pl_id++)
                    spell_overlay_id =
                        pOtherOverlayList->_4418B1(2000, pl_id + 100, 0, 65536);
                pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, 0, spell_overlay_id, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_FEATHER_FALL:  //Падение пера(пёрышком)
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 300 * spell_level;
                        break;
                    case 2:
                        spellduration = 600 * spell_level;
                        break;
                    case 3:
                        spellduration = 3600 * spell_level;
                        break;
                    case 4:
                        spellduration = 3600 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (uint pl_id = 0; pl_id < 4; pl_id++)
                    pOtherOverlayList->_4418B1(2010, pl_id + 100, 0, 65536);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 0);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 1);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 2);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 3);

                pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, 0, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_SPARKS:  // Искры
            {
                switch (skill_level) {
                    case 1:
                        amount = 3;
                        break;
                    case 2:
                        amount = 5;
                        break;
                    case 3:
                        amount = 7;
                        break;
                    case 4:
                        amount = 9;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int _v726 = (int)(60 * TrigLUT->uIntegerDoublePi) / 360;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pParty->vPosition.x, pParty->vPosition.y,
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                spell_spray_angle_start = (int)_v726 / -2;
                spell_spray_angle_end = (int)_v726 / 2;
                while (spell_spray_angle_start <= spell_spray_angle_end) {
                    pSpellSprite.uFacing = spell_spray_angle_start + (short)target_direction.uYawAngle;
                    if (pSpellSprite.Create(
                        (int16_t)(spell_spray_angle_start + (short)target_direction.uYawAngle),
                        target_direction.uPitchAngle,
                        pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed,
                        pCastSpell->uPlayerID + 1) != -1 &&
                        pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    spell_spray_angle_start += _v726 / (amount - 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_JUMP:  //Прыжок
            {
                if (pParty->uFlags & PARTY_FLAGS_1_FALLING) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        493));  // Can't cast Jump while airborne!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    break;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana))
                    break;
                for (uint pl_id = 0; pl_id < 4; pl_id++)
                    pOtherOverlayList->_4418B1(2040, pl_id + 100, 0, 65536);
                pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                pParty->uFallSpeed = 1000;
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_INVISIBILITY:  //Невидимость
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 600 * spell_level;
                        amount = spell_level;
                        break;
                    case 2:
                        spellduration = 600 * spell_level;
                        amount = 2 * spell_level;
                        break;
                    case 3:
                        spellduration = 600 * spell_level;
                        amount = 3 * spell_level;
                        break;
                    case 4:
                        spellduration = 3600 * spell_level;
                        amount = 4 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (pParty->GetRedOrYellowAlert()) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        638));  // There are hostile creatures nearby!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (pPlayer->CanCastSpell(uRequiredMana)) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 3);
                    pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(spellduration)),
                        skill_level, amount, 0, 0);
                    spell_sound_flag = true;
                }
                break;
            }

            case SPELL_AIR_FLY:  //Полёт
            {
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        494));  // Can not cast Fly indoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    break;
                }
                if (!pPlayers[pCastSpell->uPlayerID + 1]->GetMaxMana() && !engine->config->debug_all_magic) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    break;
                }
                if (skill_level == 1 || skill_level == 2 || skill_level == 3)
                    amount = 1;
                else
                    amount = 0;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (uint pl_id = 0; pl_id < 4; pl_id++)
                    pOtherOverlayList->_4418B1(2090, pl_id + 100, 0, 65536);
                spell_overlay_id =
                    pOtherOverlayList->_4418B1(10008, 203, 0, 65536);
                pParty->pPartyBuffs[PARTY_BUFF_FLY].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(3600 * spell_level)),
                    skill_level, amount, spell_overlay_id,
                    pCastSpell->uPlayerID + 1);
                spell_sound_flag = true;
                break;
            }

            case SPELL_AIR_STARBURST:  //Звездопад
            {
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        495));  // Can't cast Starburst indoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                obj_type = PID_TYPE(spell_targeted_at);
                monster_id = PID_ID(spell_targeted_at);
                if (obj_type == OBJECT_Actor) {
                    dist_X = pActors[monster_id].vPosition.x;
                    dist_Y = pActors[monster_id].vPosition.y;
                    dist_Z = pActors[monster_id].vPosition.z;
                } else {
                    dist_X = pParty->vPosition.x + 2048 * pIndoorCameraD3D->fRotationZCosine;
                    dist_Y = pParty->vPosition.y + 2048 * pIndoorCameraD3D->fRotationZSine;
                    dist_Z = pParty->vPosition.z;
                }
                unsigned __int64 k = 0;
                int j = 0;
                v730_int = obj_type == OBJECT_Actor ? spell_targeted_at : 0;
                for (uint star_num = 20; star_num; star_num--) {
                    spell_targeted_at = rand() % 1000;
                    if (sqrt(((double)spell_targeted_at + (double)dist_Z -
                              (double)(dist_Z + 2500)) *
                                 ((double)spell_targeted_at + (double)dist_Z -
                                  (double)(dist_Z + 2500)) +
                             j * j + k * k) <= 1.0) {
                        HEXRAYS_LODWORD(v685) = 0;
                        HEXRAYS_HIDWORD(v685) = 0;
                    } else {
                        HEXRAYS_HIDWORD(v685) = TrigLUT->Atan2(
                            (signed __int64)sqrt((float)(j * j + k * k)),
                            ((double)spell_targeted_at + (double)dist_Z -
                             (double)(dist_Z + 2500)));
                        HEXRAYS_LODWORD(v685) = TrigLUT->Atan2(j, k);
                    }
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.vPosition.x = dist_X;
                    pSpellSprite.vPosition.y = dist_Y;
                    pSpellSprite.vPosition.z =
                        (int)(spell_targeted_at + (dist_Z + 2500));
                    pSpellSprite.uSectorID = 0;
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = v730_int;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        stru_50C198._427546(spell_targeted_at + 2500);
                    pSpellSprite.uFacing = v685;
                    pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes = 4;
                    }
                    if (pSpellSprite.Create(v685, HEXRAYS_SHIDWORD(v685),
                                            pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed, 0) != -1 &&
                        pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                    j = rand() % 1024 - 512;
                    k = rand() % 1024 - 512;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_AWAKEN:  //Пробуждение
            {
                switch (skill_level) {
                    case 1:
                        amount = 180 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (int i = 0; i < 4; i++) {
                    if (skill_level == 4) {
                        if (pParty->pPlayers[i]
                                .conditions_times[Condition_Sleep]
                                .Valid()) {
                            pParty->pPlayers[i]
                                .conditions_times[Condition_Sleep]
                                .Reset();
                            pParty->pPlayers[i].PlaySound(SPEECH_103, 0);
                        }
                    } else {
                        if (pParty->pPlayers[i]
                                .DiscardConditionIfLastsLongerThan(
                                    Condition_Sleep,
                                    GameTime(pParty->GetPlayingTime() - GameTime::FromSeconds(amount))))
                            pParty->pPlayers[i].PlaySound(SPEECH_103, 0);
                    }
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_POISON_SPRAY:  // Распыление яда
            {
                switch (skill_level) {
                    case 1:
                        amount = 1;
                        break;
                    case 2:
                        amount = 3;
                        break;
                    case 3:
                        amount = 5;
                        break;
                    case 4:
                        amount = 7;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int _v733 = (int)(60 * TrigLUT->uIntegerDoublePi) / 360;
                if (amount == 1) {
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.vPosition.x = pParty->vPosition.x;
                    pSpellSprite.vPosition.y = pParty->vPosition.y;
                    pSpellSprite.vPosition.z = pParty->vPosition.z + (int)pParty->uPartyHeight / 3;
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pParty->vPosition.x, pParty->vPosition.y,
                        pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid = PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod = target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= 4;
                    }
                    v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                    if (pSpellSprite.Create(target_direction.uYawAngle,
                        target_direction.uPitchAngle, v659,
                        pCastSpell->uPlayerID + 1) != -1 &&
                        pParty->bTurnBasedModeOn) {
                        ++pTurnEngine->pending_actions;
                    }
                } else {
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.y = pParty->vPosition.y;
                    pSpellSprite.vPosition.x = pParty->vPosition.x;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.vPosition.z =
                        pParty->vPosition.z +
                        (signed int)pParty->uPartyHeight / 3;
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pParty->vPosition.x, pParty->vPosition.y,
                        pParty->vPosition.z +
                            (signed int)pParty->uPartyHeight / 3);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    if (pParty->bTurnBasedModeOn) {
                        pSpellSprite.uAttributes |= 4;
                    }
                    spell_spray_angle_start = _v733 / -2;
                    spell_spray_angle_end = _v733 / 2;
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
                            spell_spray_angle_start += _v733 / (amount - 1);
                        } while (spell_spray_angle_start <=
                                 spell_spray_angle_end);
                    }
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_WATER_WALK:  //Хождение по воде
            {
                if (!pPlayers[pCastSpell->uPlayerID + 1]->GetMaxMana()) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    break;
                }
                switch (skill_level) {
                    case 1:  // break;
                    case 2:
                        spellduration = 600 * spell_level;
                        break;
                    case 3:
                    case 4:
                        spellduration = 3600 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_overlay_id =
                    pOtherOverlayList->_4418B1(10005, 201, 0, 65536);
                spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 0);
                spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 1);
                spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 2);
                spell_fx_renderer->SetPlayerBuffAnim(pCastSpell->uSpellID, 3);
                pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, spell_overlay_id,
                    pCastSpell->uPlayerID + 1);
                if (skill_level == 4)
                    pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags = 1;
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_RECHARGE_ITEM:  //Перезарядка
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                v730c = &pParty->pPlayers[pCastSpell->uPlayerID_2]
                             .pInventoryItemList[spell_targeted_at];
                if (v730c->GetItemEquipType() != 12 || v730c->uAttributes & 2) {
                    _50C9D0_AfterEnchClickEventId = 113;
                    _50C9D4_AfterEnchClickEventSecondParam = 0;
                    _50C9D8_AfterEnchClickEventTimeout = 1;
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (skill_level == 1 || skill_level == 2)
                    spell_recharge_factor =
                        (double)spell_level * 0.0099999998 + 0.5;  // 50 %
                else if (skill_level == 3)
                    spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.69999999;  // 30 %
                else if (skill_level == 4)
                    spell_recharge_factor = (double)spell_level * 0.0099999998 + 0.80000001;  // 20 %
                else
                    spell_recharge_factor = 0.0;
                if (spell_recharge_factor > 1.0) spell_recharge_factor = 1.0;
                int uNewCharges = v730c->uMaxCharges * spell_recharge_factor;
                v730c->uMaxCharges = uNewCharges;
                v730c->uNumCharges = uNewCharges;
                if (uNewCharges <= 0) {
                    v730c = 0;
                    _50C9D0_AfterEnchClickEventId = 113;
                    _50C9D4_AfterEnchClickEventSecondParam = 0;
                    _50C9D8_AfterEnchClickEventTimeout = 1;
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        428));  // Spell failed     Не получилось!!!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    spell_level = spell_level;
                    continue;
                }
                v730c->uAttributes |= 0x40u;
                _50C9A8_item_enchantment_timer = 256;
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_ENCHANT_ITEM: {  // Талисман
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                uRequiredMana = 0;
                amount = 10 * spell_level;
                bool item_not_broken = true;
                int rnd = rand() % 100;
                pPlayer = &pParty->pPlayers[pCastSpell->uPlayerID_2];
                spell_item_to_enchant = &pPlayer->pInventoryItemList[spell_targeted_at - 1];
                ItemDesc *_v725 = &pItemsTable->pItems[spell_item_to_enchant->uItemID];
                char this_equip_type = _v725->uEquipType;

                // refs
                // https://www.gog.com/forum/might_and_magic_series/a_little_enchant_item_testing_in_mm7
                // http://www.pottsland.com/mm6/enchant.shtml
                // also see STDITEMS.tx and SPCITEMS.txt in Events.lod

                if ((skill_level == 1 || skill_level == 2)) __debugbreak();

                if ((skill_level == 3 || skill_level == 4) &&
                    spell_item_to_enchant->uItemID <= 134 &&
                    spell_item_to_enchant->special_enchantment == 0 &&
                    spell_item_to_enchant->uEnchantmentType == 0 &&
                    spell_item_to_enchant->m_enchantmentStrength == 0 &&
                    !spell_item_to_enchant->IsBroken()) {  // требования к предмету
                    // break items with low value
                    if ((spell_item_to_enchant->GetValue() < 450 && this_equip_type > EQUIP_BOW) ||  // armour
                        (spell_item_to_enchant->GetValue() < 250 && this_equip_type >= EQUIP_SINGLE_HANDED && this_equip_type <= EQUIP_BOW)) {  // weapons
                        if (!(spell_item_to_enchant->uAttributes & ITEM_HARDENED)) {  // предмет не сломан
                            spell_item_to_enchant->uAttributes |= ITEM_BROKEN;  //теперь сломан
                        }
                        item_not_broken = false;
                    } else {
                        // random item break
                        if (rnd >= 10 * spell_level) {  // шанс на срабатывание
                            if (!(spell_item_to_enchant->uAttributes & ITEM_HARDENED))  // предмет не сломан
                                spell_item_to_enchant->uAttributes |= ITEM_BROKEN;  //теперь сломан
                        } else {
                            if (rnd < 80) {
                                if (this_equip_type >= EQUIP_ARMOUR && this_equip_type <= EQUIP_AMULET) {  // armour gets std enchantments
                                    // ARMOUR, SHIELD, EQUIP_HELMET, BELT, CLOAK, GAUNTLETS, BOOTS, RING, AMULET

                                    int ench_found = 0;
                                    int to_item_apply_sum = 0;
                                    int ench_array[100] = { 0 };

                                    // finds how many possible enchaments and adds up to item apply values
                                    // if (pItemsTable->pEnchantments_count > 0) {
                                        for (int norm_ench_loop = 0; norm_ench_loop < 24; ++norm_ench_loop) {
                                            char* this_bon_state = pItemsTable->pEnchantments[norm_ench_loop].pBonusStat;
                                            if (this_bon_state != NULL && (this_bon_state[0] != '\0')) {
                                                int this_to_apply = pItemsTable->pEnchantments[norm_ench_loop].to_item[this_equip_type - 3];
                                                to_item_apply_sum += this_to_apply;
                                                if (this_to_apply) {
                                                    ench_array[ench_found] = norm_ench_loop;
                                                    ench_found++;
                                                }
                                            }
                                        }
                                    // }

                                    // pick a random ench
                                    int item_apply_rand = rand() % to_item_apply_sum;
                                    int target_item_apply_rand = item_apply_rand + 1;
                                    int current_item_apply_sum = 0;
                                    int step = 0;

                                    // step through until we hit that ench
                                    for (step = 0; step < ench_found; step++) {
                                        current_item_apply_sum += pItemsTable->pEnchantments[ench_array[step]].to_item[this_equip_type - 3];
                                        if (current_item_apply_sum >= target_item_apply_rand) break;
                                    }

                                    // assign ench and power
                                    spell_item_to_enchant->uEnchantmentType = (ITEM_ENCHANTMENT)ench_array[step];

                                    int ench_power = 0;
                                    // master 3-8  - guess work needs checking
                                    if (skill_level == 3) ench_power = (rand() % 6) + 3;
                                    // gm 6-12   - guess work needs checking
                                    if (skill_level == 4) ench_power = (rand() % 7) + 6;

                                    spell_item_to_enchant->m_enchantmentStrength = ench_power;
                                    spell_item_to_enchant->uAttributes |= ITEM_AURA_EFFECT_BLUE;
                                    _50C9A8_item_enchantment_timer = 256;
                                    spell_sound_flag = true;
                                    break;
                                } else if (skill_level == 3 || skill_level == 4) {  // for weapons - get the special enchantments
                                    int ench_found = 0;
                                    int to_item_apply_sum = 0;
                                    int ench_array[100] = { 0 };

                                    // finds how many possible enchaments and adds up to item apply values
                                    if (pItemsTable->pSpecialEnchantments_count > 0) {
                                        for (int spec_ench_loop = 0; spec_ench_loop < pItemsTable->pSpecialEnchantments_count; ++spec_ench_loop) {
                                            char *this_bon_state = pItemsTable->pSpecialEnchantments[spec_ench_loop].pBonusStatement;
                                            if (this_bon_state != NULL && (this_bon_state[0] != '\0')) {
                                                if (pItemsTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel == 3) continue;
                                                if (skill_level == 3 && (pItemsTable->pSpecialEnchantments[spec_ench_loop].iTreasureLevel != 0)) continue;
                                                int this_to_apply = pItemsTable->pSpecialEnchantments[spec_ench_loop].to_item_apply[this_equip_type];
                                                to_item_apply_sum += this_to_apply;
                                                if (this_to_apply) {
                                                    ench_array[ench_found] = spec_ench_loop;
                                                    ench_found++;
                                                }
                                            }
                                        }
                                    }

                                    // pick a random ench
                                    int item_apply_rand = rand() % to_item_apply_sum;
                                    int target_item_apply_rand = item_apply_rand + 1;
                                    int current_item_apply_sum = 0;
                                    int step = 0;

                                    // step through until we hit that ench
                                    for (step = 0; step < ench_found; step++) {
                                        current_item_apply_sum += pItemsTable->pSpecialEnchantments[ench_array[step]].to_item_apply[this_equip_type];
                                        if (current_item_apply_sum >= target_item_apply_rand) break;
                                    }

                                    // set item ench
                                    spell_item_to_enchant->special_enchantment = (ITEM_ENCHANTMENT)ench_array[step];
                                    spell_item_to_enchant->uAttributes |= ITEM_AURA_EFFECT_BLUE;
                                    _50C9A8_item_enchantment_timer = 256;
                                    spell_sound_flag = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (spell_sound_flag == 0) {
                    v317 = localization->GetString(428);  // Spell failed    Не получилось!!!
                    if (item_not_broken == false)
                        v317 = localization->GetString(585);  // Item too lame     Предмет недостаточно
                                   // высокого качества
                    GameUI_StatusBar_OnEvent(v317, 2);
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    // v318 =  &pParty->pPlayers[pCastSpell->uPlayerID_2];
                    pCastSpell->uSpellID = 0;
                    pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(SPEECH_43, 0);
                }
                break;
            }

            case SPELL_WATER_TOWN_PORTAL:  //Портал города
            {
                amount = 10 * spell_level;
                if (pPlayer->sMana < (signed int)uRequiredMana) break;
                if (pParty->uFlags & (PARTY_FLAGS_1_ALERT_RED |
                                      PARTY_FLAGS_1_ALERT_YELLOW) &&
                        skill_level != 4 ||
                    rand() % 100 >= amount && skill_level != 4) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                town_portal_caster_id = pCastSpell->uPlayerID;
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OnCastTownPortal, 0,
                                                    0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_WATER_LLOYDS_BEACON: {
                if (pCurrentMapName == "d05.blv") {  // Arena
                    GameUI_StatusBar_OnEvent(localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (pPlayer->sMana >= (signed int)uRequiredMana) {
                    pEventTimer->Pause();
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OnCastLloydsBeacon, 0, 0);
                    lloyds_beacon_spell_level = (signed int)(604800 * spell_level);
                    _506348_current_lloyd_playerid = pCastSpell->uPlayerID;
                    ::uRequiredMana = uRequiredMana;
                    ::sRecoveryTime = sRecoveryTime;
                    lloyds_beacon_sound_id = pCastSpell->sound_id;
                    lloyds_beacon_spell_id = pCastSpell->uSpellID;
                    pCastSpell->uFlags |= ON_CAST_NoRecoverySpell;
                }
                break;
            }

            case SPELL_EARTH_STONE_TO_FLESH: {
                switch (skill_level) {
                    case 1:
                        amount = 3600 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Pertified]
                        .Valid()) {
                    if (skill_level == 4) {  // for GM
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Pertified]
                            .Reset();
                        spell_sound_flag = true;
                        break;
                    }

                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .DiscardConditionIfLastsLongerThan(
                            Condition_Pertified,
                            GameTime(pParty->GetPlayingTime() -
                                GameTime::FromSeconds(amount)));
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_EARTH_ROCK_BLAST: {  // Взрыв камня
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pParty->vPosition.x, pParty->vPosition.y,
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uFacing = pParty->sRotationZ;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(pParty->sRotationZ, pParty->sRotationX,
                    v659,
                    pCastSpell->uPlayerID + 1) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_EARTH_DEATH_BLOSSOM: {
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pSpellSprite.uType = SPRITE_SPELL_EARTH_DEATH_BLOSSOM;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.uFacing = (short)pParty->sRotationZ;
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes = 4;
                }
                v659 = pObjectList->pObjects[pSpellSprite.uObjectDescID].uSpeed;
                if (pSpellSprite.Create(pParty->sRotationZ, TrigLUT->uIntegerHalfPi / 2, v659, 0) != -1 &&
                    pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_DETECT_LIFE:  // Детектор жизни
            {
                switch (skill_level) {
                    case 1:
                        amount = 600 * spell_level;
                        break;
                    case 2:
                        amount = 1800 * spell_level;
                        break;
                    case 3:
                    case 4:
                        amount = 3600 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                    skill_level, 0, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_FATE:  // Судьба
            {
                switch (skill_level) {
                    case 1:
                        amount = spell_level;
                        break;
                    case 2:
                        amount = 2 * spell_level;
                        break;
                    case 3:
                        amount = 4 * spell_level;
                        break;
                    case 4:
                        amount = 6 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                // LODWORD(spellduration) = 300;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (pCastSpell->spell_target_pid == 0) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .pPlayerBuffs[PLAYER_BUFF_FATE]
                        .Apply(
                            GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(5)),
                            skill_level, amount, 0, 0);
                    spell_sound_flag = true;
                    break;
                }
                if (PID_TYPE(pCastSpell->spell_target_pid) == OBJECT_Actor) {
                    monster_id = PID_ID(pCastSpell->spell_target_pid);
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_FATE].Apply(
                        GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(5)),
                        skill_level, amount, 0, 0);
                    pActors[monster_id].uAttributes |= ACTOR_AGGRESSOR;
                    spell_fx_renderer
                        ->_4A7E89_sparkles_on_actor_after_it_casts_buff(
                            &pActors[monster_id], 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_REMOVE_CURSE:  // Снятие порчи
            {
                switch (skill_level) {
                    case 1:
                        amount = 3600 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                         .conditions_times[Condition_Cursed]
                         .Valid()) {
                    spell_sound_flag = true;
                    break;
                }
                if (skill_level == 4) {  // GM
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Cursed]
                        .Reset();
                } else {
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .DiscardConditionIfLastsLongerThan(
                            Condition_Cursed,
                            GameTime(pParty->GetPlayingTime() -
                                GameTime::FromSeconds(amount)));
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Cursed]
                            .Valid()) {
                        spell_sound_flag = true;
                        break;
                    }
                }
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_PRESERVATION:  //Сохранение
            {
                if (skill_level == 4)
                    spellduration = 900 * (spell_level + 4);
                else
                    spellduration = 300 * (spell_level + 12);
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (skill_level == 1 || skill_level == 2) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(spellduration)),
                               skill_level, 0, 0, 0);
                    spell_sound_flag = true;
                    break;
                }
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_id);
                    pParty->pPlayers[pl_id]
                        .pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(spellduration)),
                               skill_level, 0, 0, 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_TURN_UNDEAD:  //Бег мертвецов
            {
                if (skill_level == 1 || skill_level == 2)
                    spellduration = 60 * (spell_level + 3);
                else
                    spellduration = 300 * spell_level + 180;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int mon_num = render->GetActorsInViewport(4096);
                spell_fx_renderer
                    ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(0xFFFFFF,
                                                                     192);
                // ++pSpellSprite.uType;
                pSpellSprite.uType = SPRITE_SPELL_SPIRIT_TURN_UNDEAD_1;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
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
                                       skill_level, 0, 0, 0);
                    }
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_RAISE_DEAD:  //Оживление
            {
                if (skill_level == 4)
                    amount = 0;
                else
                    amount = 86400 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pOtherOverlayList->_4418B1(5080, pCastSpell->uPlayerID_2 + 100,
                                           0, 65536);
                if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                         .conditions_times[Condition_Dead]
                         .Valid()) {
                    spell_sound_flag = true;
                    break;
                }
                pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                if (skill_level == 4) {
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Dead]
                        .Reset();
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Unconcious]
                        .Reset();
                } else {
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .DiscardConditionIfLastsLongerThan(
                            Condition_Dead, GameTime(pParty->GetPlayingTime() -
                                                GameTime::FromSeconds(amount)));
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .DiscardConditionIfLastsLongerThan(
                            Condition_Unconcious,
                            GameTime(pParty->GetPlayingTime() -
                                GameTime::FromSeconds(amount)));
                }
                pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                    Condition_Weak, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_SHARED_LIFE:  //Общая жизнь
            {
                if (skill_level == 4)
                    amount = 4 * spell_level;
                else
                    amount = 3 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int active_pl_num = 0;
                signed int shared_life_count = amount;
                int mean_life = 0;
                int pl_array[4];
                for (uint pl_id = 1; pl_id <= 4; pl_id++) {
                    if (!pPlayers[pl_id]
                             ->conditions_times[Condition_Dead]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Pertified]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Eradicated]
                             .Valid())
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
                spell_sound_flag = true;
                break;
            }

            case SPELL_SPIRIT_RESSURECTION:  //Воскресение
            {
                switch (skill_level) {
                    case 1:
                        amount = 180 * spell_level;
                        break;
                    case 2:
                        amount = 10800 * spell_level;
                        break;
                    case 3:
                        amount = 259200 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Eradicated]
                        .Valid() ||
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Dead]
                        .Valid()) {
                    if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                             .conditions_times[Condition_Weak]
                             .Valid())
                        pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(
                            SPEECH_25, 0);
                    if (skill_level == 4) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Eradicated]
                            .Reset();
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Dead]
                            .Reset();
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Unconcious]
                            .Reset();
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
                                Condition_Unconcious,
                                GameTime(pParty->GetPlayingTime() -
                                    GameTime::FromSeconds(amount)));
                    }
                    pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                        Condition_Weak, 1);
                    pParty->pPlayers[pCastSpell->uPlayerID_2].sHealth = 1;
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_CURE_PARALYSIS:  //лечение паралича
            {
                switch (skill_level) {
                    case 1:
                        amount = 3600 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                         .conditions_times[Condition_Paralyzed]
                         .Valid()) {
                    spell_sound_flag = true;
                    break;
                }
                if (skill_level == 4) {
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Paralyzed]
                        .Reset();
                    spell_sound_flag = true;
                    break;
                }

                pParty->pPlayers[pCastSpell->uPlayerID_2]
                    .DiscardConditionIfLastsLongerThan(
                        Condition_Paralyzed, GameTime(pParty->GetPlayingTime() -
                                                 GameTime::FromSeconds(amount)));

                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_REMOVE_FEAR:  //Снять страх
            {
                switch (skill_level) {
                    case 1:
                        amount = 180 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                         .conditions_times[Condition_Fear]
                         .Valid()) {
                    spell_sound_flag = true;
                    break;
                }
                if (skill_level == 4) {
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Fear]
                        .Reset();
                    spell_sound_flag = true;
                    break;
                }

                pParty->pPlayers[pCastSpell->uPlayerID_2]
                    .DiscardConditionIfLastsLongerThan(
                        Condition_Fear, GameTime(pParty->GetPlayingTime() -
                                            GameTime::FromSeconds(amount)));

                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_TELEPATHY:  //Телепатия
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    monster_id = PID_ID(spell_targeted_at);
                    if (!pActors[monster_id].ActorHasItem())
                        pActors[monster_id].SetRandomGoldIfTheresNoItem();
                    int gold_num = 0;
                    if (pItemsTable
                            ->pItems
                                [pActors[monster_id].ActorHasItems[3].uItemID]
                            .uEquipType == EQUIP_GOLD)
                        gold_num = pActors[monster_id]
                                       .ActorHasItems[3]
                                       .special_enchantment;
                    ItemGen item;
                    item.Reset();
                    if (pActors[monster_id].uCarriedItemID) {
                        item.uItemID = pActors[monster_id].uCarriedItemID;
                    } else {
                        for (uint i = 0; i < 4; ++i) {
                            if (pActors[monster_id].ActorHasItems[i].uItemID >
                                    0 &&
                                pItemsTable
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
                        if (item.uItemID)
                            GameUI_StatusBar_OnEvent(StringPrintf(
                                "(%s), and %d gold",
                                item.GetDisplayName().c_str(), gold_num));
                        else
                            GameUI_StatusBar_OnEvent(
                                StringPrintf("%d gold", gold_num));
                    } else {
                        if (item.uItemID) {
                            GameUI_StatusBar_OnEvent(StringPrintf(
                                "(%s)", item.GetDisplayName().c_str()));
                        } else {
                            GameUI_StatusBar_OnEvent("nothing");
                        }
                    }

                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].uActorHeight;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID = pIndoor->GetSector(
                        pSpellSprite.vPosition.x, pSpellSprite.vPosition.y,
                        pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= 0x80;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_BERSERK:  //Берсерк
            {
                switch (skill_level) {
                    case 1:
                        amount = 300 * spell_level;
                        break;
                    case 2:
                        amount = 300 * spell_level;
                        break;
                    case 3:
                        amount = 600 * spell_level;
                        break;
                    case 4:
                        amount = 3600;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                monster_id = PID_ID(spell_targeted_at);
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    // v730 = 836 * monster_id;
                    if (pActors[monster_id].DoesDmgTypeDoDamage(
                            (DAMAGE_TYPE)7)) {
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
                                   skill_level, 0, 0, 0);
                        pActors[monster_id].pMonsterInfo.uHostilityType =
                            MonsterInfo::Hostility_Long;
                    }
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z +
                                               pActors[monster_id].uActorHeight;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID =
                        pIndoor->GetSector(pActors[monster_id].vPosition.x,
                                           pActors[monster_id].vPosition.y,
                                           pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= 0x80;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_ENSLAVE:  //Порабощение
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                amount = 600 * spell_level;
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    monster_id = PID_ID(spell_targeted_at);
                    // v730 = 836 * monster_id;
                    if (MonsterStats::BelongsToSupertype(
                            pActors[monster_id].pMonsterInfo.uID,
                            MONSTER_SUPERTYPE_UNDEAD))
                        break;
                    if (pActors[monster_id].DoesDmgTypeDoDamage(
                            (DAMAGE_TYPE)7)) {
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
                                   skill_level, 0, 0, 0);
                    }
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z +
                                               pActors[monster_id].uActorHeight;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID =
                        pIndoor->GetSector(pActors[monster_id].vPosition.x,
                                           pActors[monster_id].vPosition.y,
                                           pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= 0x80;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_MASS_FEAR: {  // Массовый страх
                if (skill_level == 4)
                    amount = 300 * spell_level;
                else
                    amount = 180 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int mon_num = render->GetActorsInViewport(4096);
                spell_fx_renderer
                    ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(0xA0A0A,
                                                                     192);
                // ++pSpellSprite.uType;
                pSpellSprite.uType = SPRITE_SPELL_MIND_MASS_FEAR_1;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                for (spell_targeted_at = 0; spell_targeted_at < mon_num;
                     ++spell_targeted_at) {
                    if (MonsterStats::BelongsToSupertype(
                            pActors[_50BF30_actors_in_viewport_ids
                                        [spell_targeted_at]]
                                .pMonsterInfo.uID,
                            MONSTER_SUPERTYPE_UNDEAD))
                        break;
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
                                .DoesDmgTypeDoDamage((DAMAGE_TYPE)7)) {
                        pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                                .pActorBuffs[ACTOR_BUFF_AFRAID]
                                .Apply(GameTime(pParty->GetPlayingTime() +
                                           GameTime::FromSeconds(amount)),
                                       skill_level, 0, 0, 0);
                    }
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_MIND_CURE_INSANITY:  //Лечение Безумия
            {
                if (skill_level == 4)
                    amount = 0;
                else
                    amount = 86400 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Insane]
                        .Valid()) {
                    if (!pParty->pPlayers[pCastSpell->uPlayerID_2]
                             .conditions_times[Condition_Weak]
                             .Valid())
                        pParty->pPlayers[pCastSpell->uPlayerID_2].PlaySound(
                            SPEECH_25, 0);
                    if (skill_level == 4)
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Insane]
                            .Reset();
                    else
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .DiscardConditionIfLastsLongerThan(
                                Condition_Insane,
                                GameTime(pParty->GetPlayingTime() -
                                    GameTime::FromSeconds(amount)));
                    pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                        Condition_Weak, 0);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_EARTH_TELEKINESIS:  //Телекинез
            {
                switch (skill_level) {
                    case 1:
                        amount = 2 * spell_level;
                        break;
                    case 2:
                        amount = 2 * spell_level;
                        break;
                    case 3:
                        amount = 3 * spell_level;
                        break;
                    case 4:
                        amount = 4 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int obj_id = PID_ID(spell_targeted_at);
                if (PID_TYPE(spell_targeted_at) == OBJECT_Item) {
                    if (pItemsTable
                            ->pItems[pSpriteObjects[obj_id]
                                         .containing_item.uItemID]
                            .uEquipType == EQUIP_GOLD) {
                        pParty->PartyFindsGold(
                            pSpriteObjects[obj_id]
                                .containing_item.special_enchantment,
                            0);
                        viewparams->bRedrawGameUI = true;
                    } else {
                        GameUI_StatusBar_OnEvent(localization->FormatString(
                            471, pItemsTable
                                     ->pItems[pSpriteObjects[obj_id]
                                                  .containing_item.uItemID]
                                     .pUnidentifiedName));  // You find %s! Вы
                                                            // нашли ^Pv[%s]!
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
                    if (pLevelDecorations[pSpriteObjects[obj_id]
                                              .containing_item.uItemID]
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
                if (PID_TYPE(spell_targeted_at) == OBJECT_BModel) {
                    OpenedTelekinesis = true;
                    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                        v448 = pIndoor
                                   ->pFaceExtras[pIndoor->pFaces[obj_id]
                                                     .uFaceExtraID]
                                   .uEventID;
                    else
                        v448 = pOutdoor->pBModels[spell_targeted_at >> 9]
                                   .pFaces[obj_id & 0x3F]
                                   .sCogTriggeredID;
                    EventProcessor(v448, spell_targeted_at, 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_CURE_WEAKNESS:  //Лечить Слабость
            {
                switch (skill_level) {
                    case 1:
                        amount = 180 * spell_level;
                        break;  // 3 минуты * количество очков навыка
                    case 2:
                        amount = 3600 * spell_level;
                        break;  // 1 час * количество очков навыка
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Weak]
                        .Valid()) {
                    if (skill_level == 4) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Weak]
                            .Reset();
                        spell_sound_flag = true;
                        break;
                    }
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .DiscardConditionIfLastsLongerThan(
                            Condition_Weak, GameTime(pParty->GetPlayingTime() -
                                                GameTime::FromSeconds(amount)));
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_FIRST_AID:  //Первая помощь
            {
                switch (skill_level) {
                    case 1:
                        amount = 2 * spell_level + 5;
                        break;
                    case 2:
                        amount = 3 * spell_level + 5;
                        break;
                    case 3:
                        amount = 4 * spell_level + 5;
                        break;
                    case 4:
                        amount = 5 * spell_level + 5;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_CURE_POISON:  //Лечение Отравления(Противоядие)
            {
                switch (skill_level) {
                    case 1:
                        amount = 3600 * spell_level;
                        break;
                    case 2:
                        amount = 3600 * spell_level;
                        break;
                    case 3:
                        amount = 86400 * spell_level;
                        break;
                    case 4:
                        amount = 0;
                        break;
                    default:
                        assert(false);
                }

                if (!pPlayer->CanCastSpell(uRequiredMana)) break;

                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Poison_Weak]
                        .Valid() ||
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Poison_Medium]
                        .Valid() ||
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .conditions_times[Condition_Poison_Severe]
                        .Valid()) {
                    if (skill_level == 4) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Poison_Weak]
                            .Reset();
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Poison_Medium]
                            .Reset();
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Poison_Severe]
                            .Reset();
                        spell_sound_flag = true;
                        break;
                    }

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
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_PROTECTION_FROM_MAGIC:  //Защита от магии
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;

                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 0);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 1);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 2);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 3);
                pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(3600 * spell_level)),
                    skill_level, spell_level, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_HAMMERHANDS:  //Руки-Молоты
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (skill_level == 4) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 3);
                    for (uint pl_id = 0; pl_id < 4; pl_id++) {
                        pParty->pPlayers[pl_id]
                            .pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                            .Apply(
                                GameTime(pParty->GetPlayingTime() +
                                    GameTime::FromSeconds(3600 * spell_level)),
                                4, spell_level, spell_level, 0);
                    }
                    spell_sound_flag = true;
                    break;
                }
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                pParty->pPlayers[pCastSpell->uPlayerID_2]
                    .pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS]
                    .Apply(GameTime(pParty->GetPlayingTime() +
                               GameTime::FromSeconds(3600 * spell_level)),
                           skill_level, spell_level, spell_level, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_BODY_POWER_CURE:  //Исцеление
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (uint pl_id = 0; pl_id < 4; ++pl_id) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_id);
                    pParty->pPlayers[pl_id].Heal(5 * spell_level + 10);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_DISPEL_MAGIC: {  // Снятие чар
                sRecoveryTime -= spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer
                    ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(0xAFF0A,
                                                                     192);
                int mon_num = render->GetActorsInViewport(4096);
                // ++pSpellSprite.uType;
                pSpellSprite.uType = SPRITE_SPELL_LIGHT_DISPEL_MAGIC_1;
                v688.x = 0;
                v688.y = 0;
                v688.z = 0;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
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
                        &v688);
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
                        (unsigned int)(signed __int64)((double)pActors
                                                           [_50BF30_actors_in_viewport_ids
                                                                [spell_targeted_at]]
                                                               .uActorHeight *
                                                       -0.8);
                    pSpellSprite.spell_target_pid =
                        PID(OBJECT_Actor,
                            _50BF30_actors_in_viewport_ids[spell_targeted_at]);
                    pSpellSprite.Create(0, 0, 0, 0);
                    for (int i = 0; i < 22; ++i)
                        pActors
                            [_50BF30_actors_in_viewport_ids[spell_targeted_at]]
                                .pActorBuffs[i]
                                .Reset();
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_SUMMON_ELEMENTAL:  //Элементал
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 300 * spell_level;
                        amount = 1;
                        break;
                    case 2:
                        spellduration = 300 * spell_level;
                        amount = 1;
                        break;
                    case 3:
                        spellduration = 900 * spell_level;
                        amount = 3;
                        break;
                    case 4:
                        spellduration = 900 * spell_level;
                        amount = 5;
                        break;
                    default:
                        assert(false);
                }
                int mon_num = 0;
                for (uint monster_id = 0; monster_id < uNumActors;
                     monster_id++) {
                    if (pActors[monster_id].uAIState != Dead &&
                        pActors[monster_id].uAIState != Removed &&
                        pActors[monster_id].uAIState != Disabled &&
                        PID(OBJECT_Player, pCastSpell->uPlayerID) ==
                            pActors[monster_id].uSummonerID)
                        ++mon_num;
                }
                if (mon_num >= amount) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        648));  // This character can't summon any more
                                // monsters!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                Spawn_Light_Elemental(pCastSpell->uPlayerID,
                                                 skill_level, spellduration);
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_DAY_OF_THE_GODS:  //День богов
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 10800 * spell_level;
                        amount = 3 * spell_level + 10;
                        break;
                    case 2:
                        spellduration = 10800 * spell_level;
                        amount = 3 * spell_level + 10;
                        break;
                    case 3:
                        spellduration = 14400 * spell_level;
                        amount = 4 * spell_level + 10;
                        break;
                    case 4:
                        spellduration = 18000 * spell_level;
                        amount = 5 * spell_level + 10;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
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
                    skill_level, amount, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_PRISMATIC_LIGHT: {  // Свет призмы
                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        497));  // Can't cast Prismatic Light outdoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int mon_num = render->GetActorsInViewport(4096);
                // ++pSpellSprite.uType;
                pSpellSprite.uType = SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT_1;
                v694.x = 0;
                v694.y = 0;
                v694.z = 0;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                for (uint monster_id = 0; monster_id < mon_num; monster_id++) {
                    pSpellSprite.vPosition.x =
                        pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                            .vPosition.x;
                    pSpellSprite.vPosition.y =
                        pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                            .vPosition.y;
                    pSpellSprite.vPosition.z =
                        pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                            .vPosition.z -
                        pActors[_50BF30_actors_in_viewport_ids[monster_id]]
                                .uActorHeight *
                            -0.8;
                    pSpellSprite.spell_target_pid =
                        PID(OBJECT_Actor,
                            _50BF30_actors_in_viewport_ids[monster_id]);
                    Actor::DamageMonsterFromParty(
                        PID(OBJECT_Item, pSpellSprite.Create(0, 0, 0, 0)),
                        _50BF30_actors_in_viewport_ids[monster_id], &v694);
                }
                // v537 = spell_fx_renderer;
                spell_fx_renderer->_4A8BFC_prismatic_light();
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_DAY_OF_PROTECTION:  //День защиты
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 14400 * spell_level;
                        amount = 4 * spell_level;
                        break;
                    case 2:
                        spellduration = 14400 * spell_level;
                        amount = 4 * spell_level;
                        break;
                    case 3:
                        spellduration = 14400 * spell_level;
                        amount = 4 * spell_level;
                        break;
                    case 4:
                        spellduration = 18000 * spell_level;
                        amount = 5 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 0);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 1);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 2);
                spell_fx_renderer->SetPlayerBuffAnim(
                    pCastSpell->uSpellID, 3);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_BODY].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_MIND].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_FIRE].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_WATER].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_AIR].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_RESIST_EARTH].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, amount, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, spell_level + 5, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(spellduration)),
                    skill_level, spell_level + 5, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_HOUR_OF_POWER:  //Час могущества
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 4;
                        amount = 4;
                        break;
                    case 2:
                        spellduration = 4;
                        amount = 4;
                        break;
                    case 3:
                        spellduration = 12;
                        amount = 12;
                        break;
                    case 4:
                        spellduration = 20;
                        amount = 15;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                bool player_weak = false;
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 0);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 1);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 2);
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, 3);
                    pParty->pPlayers[pl_id]
                        .pPlayerBuffs[PLAYER_BUFF_BLESS]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(
                                       300 * amount * spell_level + 60)),
                               skill_level, spell_level + 5, 0, 0);
                    if (pParty->pPlayers[pl_id]
                            .conditions_times[Condition_Weak]
                            .Valid())
                        player_weak = true;
                }

                pParty->pPartyBuffs[PARTY_BUFF_HEROISM].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(300 * amount * spell_level + 60)),
                    skill_level, spell_level + 5, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_SHIELD].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(300 * amount * spell_level + 60)),
                    skill_level, 0, 0, 0);
                pParty->pPartyBuffs[PARTY_BUFF_STONE_SKIN].Apply(
                    GameTime(pParty->GetPlayingTime() +
                        GameTime::FromSeconds(300 * amount * spell_level + 60)),
                    skill_level, spell_level + 5, 0, 0);
                if (!player_weak)
                    pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(
                                60 * (spell_level * spellduration + 60))),
                        skill_level, spell_level + 5, 0, 0);
                spell_sound_flag = true;
                break;
            }

            case SPELL_LIGHT_DIVINE_INTERVENTION: {
                // amount = 3;
                if (pPlayer->uNumDivineInterventionCastsThisDay >= 3) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    for (uint buff_id = 0; buff_id <= 19; buff_id++)
                        pParty->pPlayers[pl_id]
                            .conditions_times[buff_id]
                            .Reset();
                    pParty->pPlayers[pl_id].sHealth =
                        pParty->pPlayers[pl_id].GetMaxHealth();
                    pParty->pPlayers[pl_id].sMana =
                        pParty->pPlayers[pl_id].GetMaxMana();
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_id);
                }
                if (pPlayer->sAgeModifier + 10 >= 120)
                    pPlayer->sAgeModifier = 120;
                else
                    pPlayer->sAgeModifier = pPlayer->sAgeModifier + 10;
                sRecoveryTime += -5 * spell_level;
                ++pPlayer->uNumDivineInterventionCastsThisDay;
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_REANIMATE:  //Реанимация
            {
                switch (skill_level) {
                    case 1:
                        amount = 2 * spell_level;
                        break;
                    case 2:
                        amount = 3 * spell_level;
                        break;
                    case 3:
                        amount = 4 * spell_level;
                        break;
                    case 4:
                        amount = 5 * spell_level;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (!pCastSpell->spell_target_pid) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    if (pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Dead]
                            .Valid()) {
                        pParty->pPlayers[pCastSpell->uPlayerID_2].SetCondition(
                            Condition_Zombie, 1);
                        GameUI_ReloadPlayerPortraits(
                            pCastSpell->uPlayerID_2,
                            (pParty->pPlayers[pCastSpell->uPlayerID_2]
                                 .GetSexByVoice() != 0) +
                                23);
                        pParty->pPlayers[pCastSpell->uPlayerID_2]
                            .conditions_times[Condition_Zombie] =
                            pParty->GetPlayingTime();
                    }
                    break;
                }
                monster_id = PID_ID(pCastSpell->spell_target_pid);
                if (monster_id == -1) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        496));  // No valid target exists!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (pActors[monster_id].sCurrentHP > 0 ||
                    pActors[monster_id].uAIState != Dead &&
                        pActors[monster_id].uAIState != Dying) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                // ++pSpellSprite.uType;
                pSpellSprite.uType = SPRITE_SPELL_DARK_REANIMATE_1;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
                pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                pSpellSprite.vPosition.z =
                    pActors[monster_id].vPosition.z -
                    (int)((double)pActors[monster_id].uActorHeight * -0.8);
                pSpellSprite.spell_target_pid = PID(OBJECT_Actor, monster_id);
                pSpellSprite.Create(0, 0, 0, 0);
                if (pActors[monster_id].pMonsterInfo.uLevel > amount) break;
                Actor::Resurrect(monster_id);
                pActors[monster_id].pMonsterInfo.uHostilityType =
                    (MonsterInfo::HostilityRadius)0;
                pActors[monster_id].pMonsterInfo.uTreasureDropChance = 0;
                pActors[monster_id].pMonsterInfo.uTreasureDiceRolls = 0;
                pActors[monster_id].pMonsterInfo.uTreasureDiceSides = 0;
                pActors[monster_id].pMonsterInfo.uTreasureLevel = 0;
                pActors[monster_id].pMonsterInfo.uTreasureType = 0;
                pActors[monster_id].uAlly = 9999;
                pActors[monster_id].ResetAggressor();  // ~0x80000
                pActors[monster_id].uGroup = 0;
                pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Reset();
                if (pActors[monster_id].sCurrentHP > 10 * amount)
                    pActors[monster_id].sCurrentHP = 10 * amount;
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_VAMPIRIC_WEAPON:  // Оружие-вампир
            {
                amount = 16;
                if (skill_level == 4)
                    spellduration = 0;
                else
                    spellduration = 3600 * spell_level;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                ItemGen *item = &pParty->pPlayers[pCastSpell->uPlayerID_2]
                                     .pInventoryItemList[spell_targeted_at];
                item->UpdateTempBonus(pParty->GetPlayingTime());
                if (item->uItemID >= 64 && item->uItemID <= 65  // blasters
                    || item->uAttributes & 2 ||
                    item->special_enchantment != 0 ||
                    item->uEnchantmentType != 0 ||
                    pItemsTable->pItems[item->uItemID].uEquipType !=
                            EQUIP_SINGLE_HANDED &&
                        pItemsTable->pItems[item->uItemID].uEquipType !=
                            EQUIP_TWO_HANDED &&
                        pItemsTable->pItems[item->uItemID].uEquipType !=
                            EQUIP_BOW ||
                    pItemsTable->IsMaterialNonCommon(item)) {
                    _50C9D0_AfterEnchClickEventId = 113;
                    _50C9D4_AfterEnchClickEventSecondParam = 0;
                    _50C9D8_AfterEnchClickEventTimeout = 1;
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                item->special_enchantment = ITEM_ENCHANTMENT_VAMPIRIC;
                if (skill_level != 4) {
                    item->uExpireTime = GameTime(pParty->GetPlayingTime() +
                                         GameTime::FromSeconds(spellduration));
                    item->uAttributes |= 8;
                }
                item->uAttributes |= 0x80;
                _50C9A8_item_enchantment_timer = 256;
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_SHARPMETAL:  //Шрапнель
            {
                switch (skill_level) {
                    case 1:
                        amount = 5;
                        break;
                    case 2:
                        amount = 5;
                        break;
                    case 3:
                        amount = 7;
                        break;
                    case 4:
                        amount = 9;
                        break;
                    default:
                        assert(false);
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                signed int _v726 =
                    ((signed int)(60 * TrigLUT->uIntegerDoublePi) / 360);
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.vPosition.x = pParty->vPosition.x;
                pSpellSprite.vPosition.y = pParty->vPosition.y;
                pSpellSprite.uAttributes = 0;
                pSpellSprite.vPosition.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 2;
                pSpellSprite.uSectorID = pIndoor->GetSector(
                    pParty->vPosition.x, pParty->vPosition.y,
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 2);
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.spell_target_pid = spell_targeted_at;
                pSpellSprite.field_60_distance_related_prolly_lod =
                    target_direction.uDistance;
                pSpellSprite.uSoundID = pCastSpell->sound_id;
                if (pParty->bTurnBasedModeOn) {
                    pSpellSprite.uAttributes |= 4;
                }
                spell_spray_angle_start = (signed int)_v726 / -2;
                spell_spray_angle_end = (signed int)_v726 / 2;
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
                        spell_spray_angle_start += _v726 / (amount - 1);
                    } while (spell_spray_angle_start <= spell_spray_angle_end);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_CONTROL_UNDEAD:  //Глава нежити
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                switch (skill_level) {
                    case 1:
                        spellduration = 180 * spell_level;
                        break;
                    case 2:
                        spellduration = 180 * spell_level;
                        break;
                    case 3:
                        spellduration = 300 * spell_level;
                        break;
                    case 4:
                        spellduration = 29030400;
                        break;
                    default:
                        assert(false);
                }
                if (PID_TYPE(spell_targeted_at) == OBJECT_Actor) {
                    monster_id = PID_ID(spell_targeted_at);
                    if (!MonsterStats::BelongsToSupertype(
                            pActors[monster_id].pMonsterInfo.uID,
                            MONSTER_SUPERTYPE_UNDEAD))
                        break;
                    if (!pActors[monster_id].DoesDmgTypeDoDamage(
                            (DAMAGE_TYPE)10)) {
                        GameUI_StatusBar_OnEvent(
                            localization->GetString(428));  // Spell failed
                        pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1, 0, 0);
                        pCastSpell->uSpellID = 0;
                        continue;
                    }
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_BERSERK].Reset();
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
                    pActors[monster_id].pActorBuffs[ACTOR_BUFF_ENSLAVED].Apply(
                        GameTime(pParty->GetPlayingTime() +
                            GameTime::FromSeconds(spellduration)),
                        skill_level, 0, 0, 0);
                    pSpellSprite.containing_item.Reset();
                    pSpellSprite.spell_id = pCastSpell->uSpellID;
                    pSpellSprite.spell_level = spell_level;
                    pSpellSprite.spell_skill = skill_level;
                    pSpellSprite.uObjectDescID =
                        pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                    pSpellSprite.vPosition.x = pActors[monster_id].vPosition.x;
                    pSpellSprite.vPosition.y = pActors[monster_id].vPosition.y;
                    pSpellSprite.vPosition.z = pActors[monster_id].vPosition.z +
                                               pActors[monster_id].uActorHeight;
                    pSpellSprite.uAttributes = 0;
                    pSpellSprite.uSectorID =
                        pIndoor->GetSector(pActors[monster_id].vPosition.x,
                                           pActors[monster_id].vPosition.y,
                                           pSpellSprite.vPosition.z);
                    pSpellSprite.uSpriteFrameID = 0;
                    pSpellSprite.spell_caster_pid =
                        PID(OBJECT_Player, pCastSpell->uPlayerID);
                    pSpellSprite.spell_target_pid = spell_targeted_at;
                    pSpellSprite.field_60_distance_related_prolly_lod =
                        target_direction.uDistance;
                    pSpellSprite.uFacing = target_direction.uYawAngle;
                    pSpellSprite.uAttributes |= 0x80;
                    pSpellSprite.uSoundID = pCastSpell->sound_id;
                    pSpellSprite.Create(0, 0, 0, pCastSpell->uPlayerID + 1);
                }
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_SACRIFICE:  //Жертва
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                int hired_npc = 0;
                memset(&achieved_awards, 0, 4000);
                for (uint npc_id = 0; npc_id < 2;
                     npc_id++) {  // количество нанятых НПС
                    if (pParty->pHirelings[npc_id].pName != 0)
                        achieved_awards[hired_npc++] = (AwardType)(npc_id + 1);
                }

                if (pCastSpell->uPlayerID_2 != 4 &&
                        pCastSpell->uPlayerID_2 != 5 ||
                    achieved_awards[pCastSpell->uPlayerID_2 - 4] <= 0 ||
                    achieved_awards[pCastSpell->uPlayerID_2 - 4] >= 3) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                pParty
                    ->pHirelings[achieved_awards[pCastSpell->uPlayerID_2 - 4] -
                                 1]
                    .evt_A = 1;
                pParty
                    ->pHirelings[achieved_awards[pCastSpell->uPlayerID_2 - 4] -
                                 1]
                    .evt_B = 0;
                pParty
                    ->pHirelings[achieved_awards[pCastSpell->uPlayerID_2 - 4] -
                                 1]
                    .evt_C =
                    pIconsFrameTable->GetIcon("spell96")->GetAnimLength();
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    pParty->pPlayers[pl_id].sHealth =
                        pParty->pPlayers[pl_id].GetMaxHealth();
                    pParty->pPlayers[pl_id].sMana =
                        pParty->pPlayers[pl_id].GetMaxMana();
                }
                v613 = &pOutdoor->ddm;
                if (uCurrentlyLoadedLevelType != LEVEL_Outdoor)
                    v613 = &pIndoor->dlv;
                v613->uReputation += 15;
                if (v613->uReputation > 10000) v613->uReputation = 10000;
                spell_sound_flag = true;
                break;
            }
            case SPELL_DARK_PAIN_REFLECTION:  //Отражение боли
            {
                switch (skill_level) {
                    case 1:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 2:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 3:
                        spellduration = 300 * (spell_level + 12);
                        break;
                    case 4:
                        spellduration = 900 * (spell_level + 4);
                        break;
                    default:
                        assert(false);
                }
                amount = spell_level + 5;
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                if (skill_level != 3 && skill_level != 4) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pCastSpell->uPlayerID_2);
                    pParty->pPlayers[pCastSpell->uPlayerID_2]
                        .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(spellduration)),
                               skill_level, amount, 0, 0);
                    spell_sound_flag = true;
                    break;
                }
                for (uint pl_id = 0; pl_id < 4; pl_id++) {
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_id);
                    pParty->pPlayers[pl_id]
                        .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                        .Apply(GameTime(pParty->GetPlayingTime() +
                                   GameTime::FromSeconds(spellduration)),
                               skill_level, amount, 0, 0);
                }
                spell_sound_flag = true;
                break;
            }
            case SPELL_DARK_SOULDRINKER:  //Испить душу
            {
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;

                int mon_num = render->GetActorsInViewport(
                    (int64_t)pIndoorCameraD3D->GetPickDepth());
                v707.x = 0;
                v707.y = 0;
                v707.z = 0;
                pSpellSprite.containing_item.Reset();
                pSpellSprite.spell_id = pCastSpell->uSpellID;
                pSpellSprite.spell_level = spell_level;
                pSpellSprite.spell_skill = skill_level;
                pSpellSprite.uObjectDescID =
                    pObjectList->ObjectIDByItemID(pSpellSprite.uType);
                pSpellSprite.uAttributes = 0;
                pSpellSprite.uSectorID = 0;
                pSpellSprite.uSpriteFrameID = 0;
                pSpellSprite.field_60_distance_related_prolly_lod = 0;
                pSpellSprite.uFacing = 0;
                pSpellSprite.spell_caster_pid =
                    PID(OBJECT_Player, pCastSpell->uPlayerID);
                pSpellSprite.uSoundID = (short)pCastSpell->sound_id;
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
                            _50BF30_actors_in_viewport_ids[monster_id], &v707);
                    }
                }
                int pl_num = 0;
                int pl_array[4];
                for (uint pl_id = 1; pl_id <= 4; ++pl_id) {
                    if (!pPlayers[pl_id]
                             ->conditions_times[Condition_Sleep]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Paralyzed]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Unconcious]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Dead]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Pertified]
                             .Valid() &&
                        !pPlayers[pl_id]
                             ->conditions_times[Condition_Eradicated]
                             .Valid()) {
                        pl_array[pl_num++] = pl_id;
                    }
                }
                for (uint j = 0; j < pl_num; j++) {
                    pPlayers[pl_array[j]]->sHealth +=
                        (signed __int64)((double)(signed int)amount /
                                         (double)pl_num);
                    if (pPlayers[pl_array[j]]->sHealth >
                        pPlayers[pl_array[j]]->GetMaxHealth())
                        pPlayers[pl_array[j]]->sHealth =
                            pPlayers[pl_array[j]]->GetMaxHealth();
                    spell_fx_renderer->SetPlayerBuffAnim(
                        pCastSpell->uSpellID, pl_array[j]);
                }
                spell_fx_renderer
                    ->FadeScreen__like_Turn_Undead_and_mb_Armageddon(0, 64);
                spell_sound_flag = true;
                break;
            }

            case SPELL_DARK_ARMAGEDDON:  //Армагеддон
            {
                if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    GameUI_StatusBar_OnEvent(localization->GetString(
                        499));  // Can't cast Armageddon indoors!
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (skill_level == 4)
                    amount = 4;
                else
                    amount = 3;
                if (pPlayer->uNumArmageddonCasts >= amount ||
                    pParty->armageddon_timer > 0) {
                    GameUI_StatusBar_OnEvent(
                        localization->GetString(428));  // Spell failed
                    pAudioPlayer->PlaySound(SOUND_spellfail0201, 0, 0, -1,
                                            0, 0);
                    pCastSpell->uSpellID = 0;
                    continue;
                }
                if (!pPlayer->CanCastSpell(uRequiredMana)) break;
                pParty->armageddon_timer = 256;
                pParty->armageddonDamage = spell_level;
                ++pPlayer->uNumArmageddonCasts;
                if (pParty->bTurnBasedModeOn) {
                    ++pTurnEngine->pending_actions;
                }
                for (uint i = 0; i < 50; i++) {
                    v642 = rand() % 4096 - 2048;
                    v643 = rand();
                    bool bOnWater = false;
                    v732_int = GetTerrainHeightsAroundParty2(
                        v642 + pParty->vPosition.x,
                        pParty->vPosition.y + (v643 % 4096 - 2048), &bOnWater, 0);
                    SpriteObject::Drop_Item_At(
                        SPRITE_SPELL_EARTH_ROCK_BLAST,
                        v642 + pParty->vPosition.x,
                        pParty->vPosition.y + (v643 % 4096 - 2048), v732_int + 16,
                        rand() % 500 + 500, 1, 0, 0, 0);
                }
                spell_sound_flag = true;
                break;
            }
            default:
                break;
        }

        if (~pCastSpell->uFlags & ON_CAST_NoRecoverySpell) {
            if (sRecoveryTime < 0) sRecoveryTime = 0;

            pPlayer =
                &pParty
                     ->pPlayers[pCastSpell->uPlayerID];  // reset to player who
                                                         // actually cast spell

            if (pParty->bTurnBasedModeOn) {
                // v645 = sRecoveryTime;
                pParty->pTurnBasedPlayerRecoveryTimes[pCastSpell->uPlayerID] =
                    sRecoveryTime;

                pPlayer->SetRecoveryTime(sRecoveryTime);

                if (!some_active_character) pTurnEngine->ApplyPlayerAction();
            } else {
                pPlayer->SetRecoveryTime((int64_t)(flt_6BE3A4_debug_recmod1 *
                    (double)sRecoveryTime * 2.133333333333333));
            }
            pPlayer->PlaySound(SPEECH_49, 0);
        }
        if (spell_sound_flag) {
            //      pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[pCastSpell->uSpellID],
            //      0, 0, -1, 0, pCastSpell->sound_id, 0, 0);
            pAudioPlayer->PlaySpellSound(pCastSpell->uSpellID, 0);
        }

        pCastSpell->uSpellID = 0;
        // spell_level = spell_level;
        continue;
    }
}

//----- (00427DA0) --------------------------------------------------------
size_t PushCastSpellInfo(uint16_t uSpellID, uint16_t uPlayerID,
                         __int16 skill_level, uint16_t uFlags,
                         int spell_sound_id) {
    // uFlags: ON_CAST_*
    for (size_t i = 0; i < CastSpellInfoCount; i++) {
        if (!pCastSpellInfo[i].uSpellID) {
            pCastSpellInfo[i].uSpellID = uSpellID;
            pCastSpellInfo[i].uPlayerID = uPlayerID;
            if (uFlags & 0x10) pCastSpellInfo[i].uPlayerID_2 = uPlayerID;
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

//----- (00427D48) --------------------------------------------------------
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

//----- (0042777D) --------------------------------------------------------
void _42777D_CastSpell_UseWand_ShootArrow(SPELL_TYPE spell,
                                          unsigned int uPlayerID,
                                          unsigned int a4, __int16 flags,
                                          int a6) {
    unsigned __int16 v9;   // cx@16
    unsigned int v10;      // eax@18
    unsigned __int8 v11;   // sf@18
    unsigned __int8 v12;   // of@18
    unsigned __int16 v13;  // cx@21
    unsigned int v14;      // eax@23

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
    if (!(flags & 0x10)) {
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
                if (a4 && ~a4 & 0x01C0) {
                    flags |= ON_CAST_WholeParty_BigImprovementAnim;
                    break;
                } else if ((player->pActiveSkills[PLAYER_SKILL_SPIRIT] &
                            0x1C0) == 0) {
                    flags |= ON_CAST_WholeParty_BigImprovementAnim;
                    break;
                }
                break;

            case SPELL_SPIRIT_PRESERVATION:
                v9 = a4;
                if (!a4) v9 = player->pActiveSkills[PLAYER_SKILL_SPIRIT];
                // goto LABEL_18;
                v10 = SkillToMastery(v9);
                v12 = __OFSUB__(v10, 3);
                v11 = ((v10 - 3) & 0x80000000u) != 0;
                // goto LABEL_24;
                if (v11 ^ v12) {
                    flags |= ON_CAST_WholeParty_BigImprovementAnim;
                    break;
                }
                break;

            case SPELL_DARK_PAIN_REFLECTION:
                v9 = a4;
                if (!a4) v9 = player->pActiveSkills[PLAYER_SKILL_DARK];
                // LABEL_18:
                v10 = SkillToMastery(v9);
                v12 = __OFSUB__(v10, 3);
                v11 = ((v10 - 3) & 0x80000000u) != 0;
                // goto LABEL_24;
                if (v11 ^ v12) {
                    flags |= ON_CAST_WholeParty_BigImprovementAnim;
                    break;
                }
                break;

            case SPELL_BODY_HAMMERHANDS:
                v13 = a4;
                if (!a4) v13 = player->pActiveSkills[PLAYER_SKILL_BODY];
                v14 = SkillToMastery(v13);
                v12 = __OFSUB__(v14, 4);
                v11 = ((v14 - 4) & 0x80000000u) != 0;
                // LABEL_24:
                if (v11 ^ v12) {
                    flags |= ON_CAST_WholeParty_BigImprovementAnim;
                    break;
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
                flags |= ON_CAST_WholeParty_BigImprovementAnim;
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

    int result = PushCastSpellInfo(spell, uPlayerID, a4, flags, a6);
    if (result != -1) {
        if (flags & ON_CAST_WholeParty_BigImprovementAnim) {
            if (pGUIWindow_CastTargetedSpell) return;
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell(
                0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)&pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton(
                52, 422, 35, 0, 2, 0,
                UIMSG_CastSpell_Character_Big_Improvement, 0, GameKey::Digit1);
            pGUIWindow_CastTargetedSpell->CreateButton(
                165, 422, 35, 0, 2, 0,
                UIMSG_CastSpell_Character_Big_Improvement, 1, GameKey::Digit2);
            pGUIWindow_CastTargetedSpell->CreateButton(
                280, 422, 35, 0, 2, 0,
                UIMSG_CastSpell_Character_Big_Improvement, 2, GameKey::Digit3);
            pGUIWindow_CastTargetedSpell->CreateButton(
                390, 422, 35, 0, 2, 0,
                UIMSG_CastSpell_Character_Big_Improvement, 3, GameKey::Digit4);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_TargetCrosshair) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell(
                0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)&pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton(
                game_viewport_x, game_viewport_y, game_viewport_width,
                game_viewport_height, 1, 0, UIMSG_CastSpell_Shoot_Monster, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Telekenesis) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell(
                0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)&pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton(
                game_viewport_x, game_viewport_y, game_viewport_width,
                game_viewport_height, 1, 0, UIMSG_CastSpell_Telekinesis, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_Enchantment) {
            if (pGUIWindow_CastTargetedSpell) return;

            pGUIWindow_CastTargetedSpell =
                pCastSpellInfo[result].GetCastSpellInInventoryWindow();
            _50C9A0_IsEnchantingInProgress = 1;
            some_active_character = uActiveCharacter;
            pParty->PickedItem_PlaceInInventory_or_Drop();
            return;
        }
        if (flags & ON_CAST_MonsterSparkles) {
            if (pGUIWindow_CastTargetedSpell) return;
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell(
                0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)&pCastSpellInfo[result]);
            pGUIWindow_CastTargetedSpell->CreateButton(
                0x34u, 0x1A6u, 0x23u, 0, 2, 0,
                UIMSG_CastSpell_Character_Small_Improvement, 0, GameKey::Digit1);
            pGUIWindow_CastTargetedSpell->CreateButton(
                0xA5u, 0x1A6u, 0x23u, 0, 2, 0,
                UIMSG_CastSpell_Character_Small_Improvement, 1, GameKey::Digit2);
            pGUIWindow_CastTargetedSpell->CreateButton(
                0x118u, 0x1A6u, 0x23u, 0, 2, 0,
                UIMSG_CastSpell_Character_Small_Improvement, 2, GameKey::Digit3);
            pGUIWindow_CastTargetedSpell->CreateButton(
                0x186u, 0x1A6u, 0x23u, 0, 2, 0,
                UIMSG_CastSpell_Character_Small_Improvement, 3, GameKey::Digit4);
            pGUIWindow_CastTargetedSpell->CreateButton(
                8, 8, game_viewport_width, game_viewport_height, 1, 0,
                UIMSG_CastSpell_Monster_Improvement, 0);
            pParty->PickedItem_PlaceInInventory_or_Drop();
        }
        if (flags & ON_CAST_DarkSacrifice && !pGUIWindow_CastTargetedSpell) {
            pGUIWindow_CastTargetedSpell = new OnCastTargetedSpell(
                0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)&pCastSpellInfo[result]);
            pBtn_NPCLeft = pGUIWindow_CastTargetedSpell->CreateButton(
                469, 178, ui_btn_npc_left->GetWidth(),
                ui_btn_npc_left->GetHeight(), 1, 0, UIMSG_ScrollNPCPanel, 0, GameKey::None,
                "", {{ui_btn_npc_left}});
            pBtn_NPCRight = pGUIWindow_CastTargetedSpell->CreateButton(
                626, 178, ui_btn_npc_right->GetWidth(),
                ui_btn_npc_right->GetHeight(), 1, 0, UIMSG_ScrollNPCPanel, 1, GameKey::None,
                "", {{ui_btn_npc_right}});
            pGUIWindow_CastTargetedSpell->CreateButton(
                491, 149, 64, 74, 1, 0, UIMSG_HiredNPC_CastSpell, 4, GameKey::Digit5);
            pGUIWindow_CastTargetedSpell->CreateButton(
                561, 149, 64, 74, 1, 0, UIMSG_HiredNPC_CastSpell, 5, GameKey::Digit6);
        }
    }
}
