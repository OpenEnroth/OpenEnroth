#include "Engine/Objects/Actor.h"

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "../Graphics/DecalBuilder.h"
#include "../Graphics/Level/Decoration.h"
#include "../Graphics/Outdoor.h"
#include "../Graphics/Overlays.h"
#include "../Graphics/PaletteManager.h"
#include "../Graphics/Sprites.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/Vis.h"
#include "../LOD.h"
#include "../OurMath.h"
#include "../Party.h"
#include "../Spells/CastSpellInfo.h"
#include "../Tables/FactionTable.h"
#include "../TurnEngine/TurnEngine.h"
#include "../stru298.h"
#include "ObjectList.h"
#include "SpriteObject.h"

using EngineIoc = Engine_::IocContainer;

// should be injected into Actor but struct size cant be changed
static DecalBuilder *decal_builder = EngineIoc::ResolveDecalBuilder();
static SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

std::array<Actor, 500> pActors;
size_t uNumActors;

stru319 stru_50C198;  // idb

std::array<uint, 5> _4DF380_hostilityRanges = {0, 1024, 2560, 5120, 10240};

//----- (0042FB5C) --------------------------------------------------------
// True if monster should play attack animation when casting this spell.
bool ShouldMonsterPlayAttackAnim(signed int spell_id) {
    switch (spell_id) {
        case SPELL_FIRE_HASTE:
        case SPELL_AIR_SHIELD:
        case SPELL_EARTH_STONESKIN:
        case SPELL_SPIRIT_BLESS:
        case SPELL_SPIRIT_FATE:
        case SPELL_SPIRIT_HEROISM:
        case SPELL_BODY_HAMMERHANDS:
        case SPELL_BODY_POWER_CURE:
        case SPELL_LIGHT_DISPEL_MAGIC:
        case SPELL_LIGHT_DAY_OF_PROTECTION:
        case SPELL_LIGHT_HOUR_OF_POWER:
        case SPELL_DARK_PAIN_REFLECTION:
            return false;
    }
    return true;
}

//----- (0041AF52) --------------------------------------------------------
void Actor::DrawHealthBar(Actor *actor, GUIWindow *window) {
    // bar length
    unsigned int bar_length;
    if (actor->pMonsterInfo.uHP <= 25)
        bar_length = 25;
    else if (actor->pMonsterInfo.uHP < 200)
        bar_length = actor->pMonsterInfo.uHP;
    else
        bar_length = 200;

    // bar colour
    Image *bar_image = game_ui_monster_hp_green;
    if (actor->sCurrentHP <= (0.34 * actor->pMonsterInfo.uHP))
        bar_image = game_ui_monster_hp_red;
    else if (actor->sCurrentHP <= (0.67 * actor->pMonsterInfo.uHP))
        bar_image = game_ui_monster_hp_yellow;

    // how much of bar is filled
    unsigned int bar_filled_length = bar_length;
    if (actor->sCurrentHP < (int)actor->pMonsterInfo.uHP)
        bar_filled_length = (bar_length * actor->sCurrentHP) / actor->pMonsterInfo.uHP;

    // centralise for clipping and draw
    unsigned int uX = window->uFrameX + (signed int)(window->uFrameWidth - bar_length) / 2;

    render->SetUIClipRect(uX, window->uFrameY + 32, uX + bar_length, window->uFrameY + 52);
    render->DrawTextureAlphaNew(uX / 640.0f, (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_background);

    render->SetUIClipRect(uX, window->uFrameY + 32, uX + bar_filled_length,
                          window->uFrameY + 52);
    render->DrawTextureAlphaNew(uX / 640.0f, (window->uFrameY + 34) / 480.0f,
                                bar_image);

    // draw hp bar ends
    render->ResetUIClipRect();
    render->DrawTextureAlphaNew((uX - 5) / 640.0f,
                                (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_border_left);
    render->DrawTextureAlphaNew((uX + bar_length) / 640.0f,
                                (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_border_right);
}

//----- (00448A40) --------------------------------------------------------
void Actor::ToggleFlag(signed int uActorID, unsigned int uFlag, int bToggle) {
    if (uActorID >= 0 && uActorID <= (signed int)(uNumActors - 1)) {
        if (bToggle) {
            pActors[uActorID].uAttributes |= uFlag;
        } else {
            if (uFlag == 0x10000) {
                if (pActors[uActorID].uAIState == Disabled)
                    pActors[uActorID].uAIState = Standing;
            }
            pActors[uActorID].uAttributes &= ~uFlag;
        }
    }
}

//----- (00448518) --------------------------------------------------------
void sub_448518_npc_set_item(int npc, unsigned int item, int a3) {
    for (uint i = 0; i < uNumActors; i++) {
        if (pActors[uNumActors].sNPC_ID == npc) Actor::GiveItem(i, item, a3);
    }
}

//----- (004485A7) --------------------------------------------------------
void Actor::GiveItem(signed int uActorID, unsigned int uItemID,
                     unsigned int bGive) {
    if ((uActorID >= 0) &&
        (signed int)uActorID <= (signed int)(uNumActors - 1)) {
        if (bGive) {
            if (pActors[uActorID].uCarriedItemID == ITEM_NULL)
                pActors[uActorID].uCarriedItemID = uItemID;
            else if (pActors[uActorID].ActorHasItems[0].uItemID == ITEM_NULL)
                pActors[uActorID].ActorHasItems[0].uItemID = uItemID;
            else if (pActors[uActorID].ActorHasItems[1].uItemID == ITEM_NULL)
                pActors[uActorID].ActorHasItems[1].uItemID = uItemID;
        } else {
            if (pActors[uActorID].uCarriedItemID == uItemID)
                pActors[uActorID].uCarriedItemID = ITEM_NULL;
            else if (pActors[uActorID].ActorHasItems[0].uItemID == uItemID)
                pActors[uActorID].ActorHasItems[0].Reset();
            else if (pActors[uActorID].ActorHasItems[1].uItemID == uItemID)
                pActors[uActorID].ActorHasItems[1].Reset();
        }
    }
}

//----- (0040894B) --------------------------------------------------------
bool Actor::CanAct() {
    bool stoned = this->pActorBuffs[ACTOR_BUFF_STONED].Active();
    bool paralyzed = this->pActorBuffs[ACTOR_BUFF_PARALYZED].Active();
    return !(stoned || paralyzed || this->uAIState == Dying ||
             this->uAIState == Dead || this->uAIState == Removed ||
             this->uAIState == Summoned || this->uAIState == Disabled);
}

//----- (004089C7) --------------------------------------------------------
bool Actor::IsNotAlive() {
    bool stoned = this->pActorBuffs[ACTOR_BUFF_STONED].Active();
    return (stoned || (uAIState == Dying) || (uAIState == Dead) ||
            (uAIState == Removed) || (uAIState == Summoned) ||
            (uAIState == Disabled));
}

//----- (004086E9) --------------------------------------------------------
void Actor::SetRandomGoldIfTheresNoItem() {
    int v2;  // edi@1

    v2 = 0;
    if (!this->ActorHasItems[3].uItemID) {
        if (this->pMonsterInfo.uTreasureDiceRolls) {
            for (int i = 0; i < this->pMonsterInfo.uTreasureDiceRolls; i++)
                v2 += rand() % this->pMonsterInfo.uTreasureDiceSides + 1;
            if (v2) {
                this->ActorHasItems[3].uItemID = ITEM_GOLD_SMALL;
                this->ActorHasItems[3].special_enchantment =
                    (ITEM_ENCHANTMENT)v2;  // actual gold amount
            }
        }
    }
    if (rand() % 100 < this->pMonsterInfo.uTreasureDropChance) {
        if (this->pMonsterInfo.uTreasureLevel)
            pItemsTable->GenerateItem(this->pMonsterInfo.uTreasureLevel,
                                      this->pMonsterInfo.uTreasureType,
                                      &this->ActorHasItems[2]);
    }
    this->uAttributes |= ACTOR_HAS_ITEM;
}

//----- (00404AC7) --------------------------------------------------------
void Actor::AI_SpellAttack(unsigned int uActorID, AIDirection *pDir,
                           int uSpellID, int a4, unsigned int uSkillLevel) {
    Actor *actorPtr;            // esi@1
    unsigned int realPoints;    // edi@1
    unsigned int masteryLevel;  // eax@1
    int v8;                     // edi@16
    signed int v10;             // ecx@22
    int v19;                    // edi@34
    int v20;                    // eax@35
    signed int v23;             // eax@41
    int v28;                    // st6@50
    int v30;                    // esi@50
    int v31;                    // ST3C_4@51
    unsigned int v32;           // edi@51
    signed int v36;             // eax@67
    int v39;                    // ecx@75
    int v42;                    // ecx@91
    int v44;                    // ecx@100
    int v48;                    // ecx@110
    int v51;                    // ecx@130
    int v54;                    // ecx@138
    int v59;                    // edi@146
    int v61;                    // edi@146
    signed int v63;             // edi@146
    int v68;                    // edi@168
    signed int v70;             // ecx@172
    int v79;                    // edx@185
    int v80;                    // eax@185
    signed int v91;             // eax@200
    int v94;                    // ecx@208
    int v96;                    // ecx@217
    int pitch;                  // [sp+2Ch] [bp-A4h]@51
    int v114;                   // [sp+48h] [bp-88h]@41
    SpriteObject a1;            // [sp+4Ch] [bp-84h]@1
    int v116;                   // [sp+BCh] [bp-14h]@49
    int v118;                   // [sp+C4h] [bp-Ch]@29
    int v119;                   // [sp+C8h] [bp-8h]@48
    int v120;                   // [sp+CCh] [bp-4h]@1
    int spellnuma;              // [sp+D8h] [bp+8h]@29
    int spellnumb;              // [sp+D8h] [bp+8h]@48
    int spellnumc;              // [sp+D8h] [bp+8h]@50
    int spellnume;              // [sp+D8h] [bp+8h]@179
    int a1a;                    // [sp+E0h] [bp+10h]@34
    int a1c;                    // [sp+E0h] [bp+10h]@184

    actorPtr = &pActors[uActorID];
    realPoints = uSkillLevel & 0x3F;
    masteryLevel = SkillToMastery(uSkillLevel);

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_FIRE_INCINERATE:
        case SPELL_AIR_LIGHNING_BOLT:
        case SPELL_WATER_ICE_BOLT:
        case SPELL_WATER_ACID_BURST:
        case SPELL_EARTH_BLADES:
        case SPELL_EARTH_ROCK_BLAST:
        case SPELL_MIND_MIND_BLAST:
        case SPELL_MIND_PSYCHIC_SHOCK:
        case SPELL_BODY_HARM:
        case SPELL_LIGHT_LIGHT_BOLT:
        case SPELL_DARK_TOXIC_CLOUD:
        case SPELL_DARK_DRAGON_BREATH:
            a1.uType = spell_sprite_mapping[uSpellID].uSpriteType;
            a1.uObjectDescID = GetObjDescId(uSpellID);
            a1.containing_item.Reset();
            a1.spell_id = uSpellID;
            a1.spell_level = uSkillLevel;
            a1.vPosition.x = actorPtr->vPosition.x;
            a1.spell_skill = 0;
            a1.vPosition.y = actorPtr->vPosition.y;
            a1.vPosition.z = actorPtr->vPosition.z +
                             ((signed int)actorPtr->uActorHeight >> 1);
            a1.uFacing = (short)pDir->uYawAngle;
            a1.uSoundID = 0;
            a1.uAttributes = 0;
            a1.uSectorID = pIndoor->GetSector(a1.vPosition.x, a1.vPosition.y,
                                              a1.vPosition.z);
            a1.uSpriteFrameID = 0;
            a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            a1.spell_target_pid = 0;
            if ((double)pDir->uDistance < 307.2)
                a1.field_60_distance_related_prolly_lod = 0;
            else if (pDir->uDistance < 1024)
                a1.field_60_distance_related_prolly_lod = 1;
            else if (pDir->uDistance < 2560)
                a1.field_60_distance_related_prolly_lod = 2;
            else
                a1.field_60_distance_related_prolly_lod = 3;

            a1.field_61 = 2;
            v91 = a1.Create(
                pDir->uYawAngle, pDir->uPitchAngle,
                pObjectList->pObjects[(int16_t)a1.uObjectDescID].uSpeed, 0);
            if (v91 != -1) {
                pAudioPlayer->PlaySpellSound(uSpellID, PID(OBJECT_Item, v91));
                return;
            }
            return;
            break;

        case SPELL_FIRE_HASTE:
            if (masteryLevel == 1 || masteryLevel == 2)
                v39 = 60 * (realPoints + 60);
            else if (masteryLevel == 3)
                v39 = 2 * 60 * (realPoints + 20);
            else if (masteryLevel == 4)
                v39 = 3 * 60 * (realPoints + 15);
            else
                v39 = 0;
            actorPtr->pActorBuffs[ACTOR_BUFF_HASTE].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v39 / 60)),
                masteryLevel, 0, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xFF3C1Eu);
            pAudioPlayer->PlaySound((SoundID)SOUND_Haste,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_FIRE_METEOR_SHOWER:
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) return;
            v114 = pParty->vPosition.z + 2500;
            v23 = 8;
            if (masteryLevel == 2)
                v23 = 10;
            else if (masteryLevel == 3)
                v23 = 12;
            else if (masteryLevel == 4)
                v23 = 14;
            spellnumb = 0;
            v28 = 0;
            for (int i = 0; i < v23; i++) {
                v30 = rand() % 1000;
                spellnumc = v30 - 2500;
                v120 = v28 * v28;
                v119 = spellnumb * spellnumb;
                if (sqrt((float)(spellnumc * spellnumc + v119 + v120)) <= 1.0) {
                    v32 = 0;
                    pitch = 0;
                } else {
                    v31 = (signed __int64)sqrt((float)(v119 + v120));
                    v32 = stru_5C6E00->Atan2(spellnumb, (int)v28);
                    pitch = stru_5C6E00->Atan2(v31, (int)spellnumc);
                }
                a1.containing_item.Reset();
                a1.uType = spell_sprite_mapping[uSpellID].uSpriteType;
                a1.uObjectDescID = GetObjDescId(uSpellID);
                a1.spell_level = uSkillLevel;
                a1.vPosition.x = pParty->vPosition.x;
                a1.vPosition.y = pParty->vPosition.y;
                a1.vPosition.z = v30 + v114;
                a1.spell_id = SPELL_FIRE_METEOR_SHOWER;
                a1.spell_skill = 0;
                a1.uAttributes = 0;
                a1.uSectorID = 0;
                a1.uSpriteFrameID = 0;
                a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
                a1.spell_target_pid = 0;
                a1.field_60_distance_related_prolly_lod =
                    stru_50C198._427546(v30 + 2500);
                a1.uFacing = v32;
                a1.uSoundID = 0;
                if (pDir->uDistance < 307.2)
                    a1.field_60_distance_related_prolly_lod = 0;
                else if (pDir->uDistance < 1024)
                    a1.field_60_distance_related_prolly_lod = 1;
                else if (pDir->uDistance < 2560)
                    a1.field_60_distance_related_prolly_lod = 2;
                else
                    a1.field_60_distance_related_prolly_lod = 3;
                a1.field_61 = 2;
                v36 = a1.Create(
                    v32, pitch,
                    pObjectList->pObjects[(signed __int16)a1.uObjectDescID]
                        .uSpeed,
                    0);
                if (v36 != -1) {
                    pAudioPlayer->PlaySpellSound(9, PID(OBJECT_Item, v36));
                }
                spellnumb = rand() % 1024 - 512;
                v28 = rand() % 1024 - 512;
            }
            return;
            break;

        case SPELL_AIR_SPARKS:
            if (masteryLevel == 2)
                v10 = 5;
            else if (masteryLevel == 3)
                v10 = 7;
            else if (masteryLevel == 4)
                v10 = 9;
            else
                v10 = 3;
            spellnuma = (signed int)(60 * stru_5C6E00->uIntegerDoublePi) / 360;
            a1.uType = spell_sprite_mapping[uSpellID].uSpriteType;
            v118 = (signed int)(60 * stru_5C6E00->uIntegerDoublePi) / 360 /
                   (v10 - 1);
            a1.uObjectDescID = GetObjDescId(uSpellID);

            a1.containing_item.Reset();
            a1.spell_id = SPELL_AIR_SPARKS;
            a1.spell_level = uSkillLevel;
            a1.vPosition.x = actorPtr->vPosition.x;
            a1.spell_skill = 0;
            a1.vPosition.y = actorPtr->vPosition.y;
            a1.vPosition.z = actorPtr->vPosition.z +
                             ((signed int)actorPtr->uActorHeight >> 1);
            a1.uFacing = pDir->uYawAngle;
            a1.uSoundID = 0;
            a1.uAttributes = 0;
            a1.uSectorID = pIndoor->GetSector(a1.vPosition.x, a1.vPosition.y,
                                              a1.vPosition.z);
            a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            a1.uSpriteFrameID = 0;
            a1.spell_target_pid = 0;
            a1.field_60_distance_related_prolly_lod = 3;
            v19 = spellnuma / -2;
            a1a = spellnuma / 2;
            if (spellnuma / -2 > spellnuma / 2) {
                v20 = spellnuma / 2;
            } else {
                do {
                    a1.uFacing = v19 + (short)pDir->uYawAngle;
                    v20 = a1.Create(
                        (int16_t)a1.uFacing, pDir->uPitchAngle,
                        pObjectList->pObjects[(int16_t)a1.uObjectDescID]
                            .uSpeed,
                        0);
                    v19 += v118;
                } while (v19 <= a1a);
            }
            if (v20 != -1) {
                pAudioPlayer->PlaySpellSound(15, PID(OBJECT_Item, v20));
                return;
            }
            return;
            break;

        case SPELL_AIR_SHIELD:
            if (masteryLevel == 1 || masteryLevel == 2)
                v8 = 5 * 60 * realPoints + 3840;
            else if (masteryLevel == 3)
                v8 = 15 * 60 * realPoints + 3840;
            else if (masteryLevel == 4)
                v8 = 60 * 60 * (realPoints + 64);
            else
                v8 = 0;
            actorPtr->pActorBuffs[ACTOR_BUFF_SHIELD].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v8 / 60)),
                masteryLevel, 0, 0, 0);
            return;

        case SPELL_EARTH_STONESKIN:
            if (masteryLevel == 1 || masteryLevel == 2)
                v44 = 5 * 60 * realPoints + 3840;
            else if (masteryLevel == 3)
                v44 = 15 * 60 * realPoints + 3840;
            else if (masteryLevel == 4)
                v44 = 60 * 60 * (realPoints + 64);
            else
                v44 = 0;
            actorPtr->pActorBuffs[ACTOR_BUFF_STONESKIN].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v44 / 60)),
                masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0x5C310Eu);
            pAudioPlayer->PlaySound((SoundID)SOUND_Stoneskin,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0, 0);
            return;

        case SPELL_SPIRIT_BLESS:
            if (masteryLevel == 1 || masteryLevel == 2)
                v42 = 5 * 60 * realPoints + 3840;
            else if (masteryLevel == 3)
                v42 = 15 * 60 * realPoints + 3840;
            else if (masteryLevel == 4)
                v42 = 20 * 60 * realPoints + 3840;
            else
                v42 = 0;

            actorPtr->pActorBuffs[ACTOR_BUFF_BLESS].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v42 / 60)),
                masteryLevel, realPoints + 5, 0, 0);

            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xC8C805u);
            pAudioPlayer->PlaySound((SoundID)SOUND_Bless,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_SPIRIT_FATE:
            if (masteryLevel == 1 || masteryLevel == 2)
                v48 = 2 * realPoints + 40;
            else if (masteryLevel == 3)
                v48 = 3 * realPoints + 60;
            else if (masteryLevel == 4)
                v48 = 2 * (3 * realPoints + 60);
            else
                v48 = 0;

            actorPtr->pActorBuffs[ACTOR_BUFF_FATE].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(5)),
                masteryLevel, v48, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xC8C805u);
            pAudioPlayer->PlaySound((SoundID)SOUND_Fate,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_SPIRIT_HEROISM:
            if (masteryLevel == 1 || masteryLevel == 2)
                v54 = 5 * 60 * realPoints + 3840;
            else if (masteryLevel == 3)
                v54 = 15 * 60 * realPoints + 3840;
            else if (masteryLevel == 4)
                v54 = 20 * 60 * realPoints + 3840;
            else
                v54 = 0;
            actorPtr->pActorBuffs[ACTOR_BUFF_HEROISM].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v54 / 60)),
                masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xC8C805u);
            pAudioPlayer->PlaySound((SoundID)SOUND_51heroism03,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_BODY_HAMMERHANDS:
            if ((signed int)masteryLevel <= 0 || (signed int)masteryLevel > 4)
                v51 = 0;
            else
                v51 = 60 * 60 * realPoints;
            actorPtr->pActorBuffs[ACTOR_BUFF_PAIN_HAMMERHANDS].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v51 / 60)),
                masteryLevel, realPoints, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xA81376u);
            pAudioPlayer->PlaySound((SoundID)SOUND_51heroism03,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_BODY_POWER_CURE:
            actorPtr->sCurrentHP += 5 * realPoints + 10;
            if (actorPtr->sCurrentHP >= (signed int)actorPtr->pMonsterInfo.uHP)
                actorPtr->sCurrentHP = (short)actorPtr->pMonsterInfo.uHP;
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xA81376u);
            pAudioPlayer->PlaySound((SoundID)SOUND_Fate,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_LIGHT_DISPEL_MAGIC:
            for (int i = 0; i < 20; i++) pParty->pPartyBuffs[i].Reset();
            for (int i = 1; i <= 4; i++) {
                v59 = pPlayers[i]->GetParameterBonus(
                    pPlayers[i]->GetActualWillpower());
                v61 = (pPlayers[i]->GetParameterBonus(
                           pPlayers[i]->GetActualIntelligence()) +
                       v59) /
                      2;
                v63 = v61 +
                      pPlayers[i]->GetParameterBonus(
                          pPlayers[i]->GetActualLuck()) +
                      30;
                if (rand() % v63 < 30) {
                    for (uint k = 0; k < pPlayers[i]->pPlayerBuffs.size(); k++)
                        pPlayers[i]->pPlayerBuffs[k].Reset();
                    pOtherOverlayList->_4418B1(11210, i + 99, 0, 65536);
                }
            }
            pAudioPlayer->PlaySpellSound(80, PID(OBJECT_Actor, uActorID));
            return;

        case SPELL_LIGHT_DAY_OF_PROTECTION:
            if (masteryLevel == 1 || masteryLevel == 2) {
                v96 = 5 * 60 * realPoints + 128 * 30;
            } else if (masteryLevel == 3) {
                HEXRAYS_LOWORD(realPoints) = 3 * realPoints;
                v96 = 15 * 60 * (uSkillLevel & 0x3F) + 128 * 30;
            } else if (masteryLevel == 4) {
                v96 = 20 * 60 * realPoints + 128 * 30;
                HEXRAYS_LOWORD(realPoints) = 4 * realPoints;
            } else {
                HEXRAYS_LOWORD(realPoints) = uSkillLevel;
                v96 = 0;
            }
            actorPtr->pActorBuffs[ACTOR_BUFF_DAY_OF_PROTECTION].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v96 / 60)),
                masteryLevel, realPoints, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xFFFFFFu);
            pAudioPlayer->PlaySound((SoundID)SOUND_94dayofprotection03,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_LIGHT_HOUR_OF_POWER:
            if (masteryLevel == 1 || masteryLevel == 2)
                v94 = 5 * 60 * realPoints + 30 * 128;
            else if (masteryLevel == 3)
                v94 = 15 * 60 * realPoints + 30 * 128;
            else if (masteryLevel == 4)
                v94 = 20 * 60 * realPoints + 30 * 128;
            else
                v94 = 0;
            actorPtr->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v94 / 60)),
                masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0xFFFFFFu);
            pAudioPlayer->PlaySound((SoundID)SOUND_9armageddon01,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;

        case SPELL_DARK_SHARPMETAL:
            if (masteryLevel == 2)
                v70 = 5;
            else if (masteryLevel == 3)
                v70 = 7;
            else if (masteryLevel == 4)
                v70 = 9;
            else
                v70 = 3;

            spellnume = (signed int)(60 * stru_5C6E00->uIntegerDoublePi) / 360;
            a1.uType = spell_sprite_mapping[uSpellID].uSpriteType;
            v116 = (signed int)(60 * stru_5C6E00->uIntegerDoublePi) / 360 /
                   (v70 - 1);
            a1.uObjectDescID = GetObjDescId(uSpellID);
            a1.containing_item.Reset();
            a1.spell_id = uSpellID;
            a1.spell_level = uSkillLevel;
            a1.vPosition.x = actorPtr->vPosition.x;
            a1.spell_skill = 0;
            a1.vPosition.y = actorPtr->vPosition.y;
            a1.vPosition.z = actorPtr->vPosition.z +
                             ((signed int)actorPtr->uActorHeight >> 1);
            a1.uFacing = pDir->uYawAngle;
            a1.uSoundID = 0;
            a1.uAttributes = 0;
            a1.uSectorID = pIndoor->GetSector(a1.vPosition.x, a1.vPosition.y,
                                              a1.vPosition.z);
            a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            a1.uSpriteFrameID = 0;
            a1.spell_target_pid = 0;
            a1.field_60_distance_related_prolly_lod = 3;
            a1c = spellnume / -2;
            if (spellnume / -2 > spellnume / 2) {
                v80 = spellnume / -2;
            } else {
                do {
                    v79 = pDir->uYawAngle;
                    a1.uFacing = a1c + (short)pDir->uYawAngle;
                    v80 = a1.Create(
                        v79, pDir->uPitchAngle,
                        pObjectList->pObjects[(int16_t)a1.uObjectDescID]
                            .uSpeed,
                        0);
                    a1c += v116;
                } while (a1c <= spellnume / 2);
            }
            if (v80 != -1) {
                pAudioPlayer->PlaySpellSound(93, PID(OBJECT_Item, v80));
                return;
            }
            return;

        case SPELL_DARK_PAIN_REFLECTION:
            if (masteryLevel == 0)
                v68 = 0;
            else if (masteryLevel == 1 || (masteryLevel == 2) ||
                     (masteryLevel == 3))
                v68 = 5 * 30 * realPoints + 30 * 128;
            else
                v68 = 15 * 30 * realPoints + 30 * 128;
            actorPtr->pActorBuffs[ACTOR_BUFF_PAIN_REFLECTION].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromMinutes(v68 / 60)),
                masteryLevel, 0, 0, 0);
            spell_fx_renderer->_4A7E89_sparkles_on_actor_after_it_casts_buff(actorPtr, 0x7E7E7Eu);
            pAudioPlayer->PlaySound((SoundID)SOUND_Sacrifice2,
                                    PID(OBJECT_Actor, uActorID), 0, -1, 0,
                                    0);
            return;
    }
}

unsigned short Actor::GetObjDescId(int spellId) {
    return pObjectList->ObjectIDByItemID(spell_sprite_mapping[spellId].uSpriteType);  // crash here
}

bool Actor::ArePeasantsOfSameFaction(Actor *a1, Actor *a2) {
    unsigned int v2 = a1->uAlly;
    if (!a1->uAlly) v2 = (a1->pMonsterInfo.uID - 1) / 3 + 1;

    unsigned int v3 = a2->uAlly;
    if (!a2->uAlly) v3 = (a2->pMonsterInfo.uID - 1) / 3 + 1;

    if (v2 >= 39 && v2 <= 44 && v3 >= 39 && v3 <= 44 ||
        v2 >= 45 && v2 <= 50 && v3 >= 45 && v3 <= 50 ||
        v2 >= 51 && v2 <= 62 && v3 >= 51 && v3 <= 62 ||
        v2 >= 78 && v2 <= 83 && v3 >= 78 && v3 <= 83 || v2 == v3)
        return true;
    else
        return false;
}

//----- (0043AC45) --------------------------------------------------------
void Actor::AggroSurroundingPeasants(unsigned int uActorID, int a2) {
    int v4;  // ebx@8
    int v5;  // ST1C_4@8
    int v6;  // eax@8

    int x = 0;
    x |= 0x80000;
    int y = 0;
    y |= 0x80000;
    Actor *victim = &pActors[uActorID];
    if (a2 == 1) victim->uAttributes |= ACTOR_AGGRESSOR;

    for (uint i = 0; i < uNumActors; ++i) {
        Actor *actor = &pActors[i];
        if (!actor->CanAct() || i == uActorID) continue;

        if (Actor::ArePeasantsOfSameFaction(victim, actor)) {
            v4 = abs(actor->vPosition.x - victim->vPosition.x);
            v5 = abs(actor->vPosition.y - victim->vPosition.y);
            v6 = abs(actor->vPosition.z - victim->vPosition.z);
            if (int_get_vector_length(v4, v5, v6) < 4096) {
                actor->pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
                if (a2 == 1) actor->uAttributes |= ACTOR_AGGRESSOR;
            }
        }
    }
}

//----- (00404874) --------------------------------------------------------
void Actor::AI_RangedAttack(unsigned int uActorID, struct AIDirection *pDir,
                            int type, char a4) {
    char specAb;  // al@1
    int v13;      // edx@28

    SpriteObject a1;  // [sp+Ch] [bp-74h]@1

    switch (type) {
        case 1:
            a1.uType = SPRITE_PROJECTILE_545;
            break;
        case 2:
            a1.uType = SPRITE_PROJECTILE_550;
            break;
        case 3:
            a1.uType = SPRITE_PROJECTILE_510;
            break;
        case 4:
            a1.uType = SPRITE_PROJECTILE_500;
            break;
        case 5:
            a1.uType = SPRITE_PROJECTILE_515;
            break;
        case 6:
            a1.uType = SPRITE_PROJECTILE_505;
            break;
        case 7:
            a1.uType = SPRITE_PROJECTILE_530;
            break;
        case 8:
            a1.uType = SPRITE_PROJECTILE_525;
            break;
        case 9:
            a1.uType = SPRITE_PROJECTILE_520;
            break;
        case 10:
            a1.uType = SPRITE_PROJECTILE_535;
            break;
        case 11:
            a1.uType = SPRITE_PROJECTILE_540;
            break;
        case 13:
            a1.uType = SPRITE_PROJECTILE_555;
            break;
        default:
            return;
    }
    bool found = false;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    if (a1.uObjectDescID == 0) {
        Error("Item not found");
        return;
    }
    a1.containing_item.Reset();
    a1.spell_id = 0;
    a1.vPosition.x = pActors[uActorID].vPosition.x;
    a1.vPosition.y = pActors[uActorID].vPosition.y;
    a1.vPosition.z = pActors[uActorID].vPosition.z + (pActors[uActorID].uActorHeight * 0.75);
    a1.spell_level = 0;
    a1.spell_skill = 0;
    a1.uFacing = pDir->uYawAngle;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID =
        pIndoor->GetSector(a1.vPosition.x, a1.vPosition.y, a1.vPosition.z);
    a1.uSpriteFrameID = 0;
    a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
    a1.spell_target_pid = 0;
    if (pDir->uDistance < 307.2)
        a1.field_60_distance_related_prolly_lod = 0;
    else if (pDir->uDistance < 1024)
        a1.field_60_distance_related_prolly_lod = 1;
    else if (pDir->uDistance < 2560)
        a1.field_60_distance_related_prolly_lod = 2;
    else
        a1.field_60_distance_related_prolly_lod = 3;

    a1.field_61 = a4;
    a1.Create(pDir->uYawAngle, pDir->uPitchAngle,
              pObjectList->pObjects[(signed __int16)a1.uObjectDescID].uSpeed,
              0);
    if (pActors[uActorID].pMonsterInfo.uSpecialAbilityType == 1) {
        specAb = pActors[uActorID].pMonsterInfo.uSpecialAbilityDamageDiceBonus;
        if (specAb == 2) {
            a1.vPosition.z += 40;
            v13 = pDir->uYawAngle;
        } else {
            if (specAb != 3) return;
            a1.Create(
                pDir->uYawAngle + 30,  // TODO(_) find out why the YawAngle change
                pDir->uPitchAngle,
                pObjectList->pObjects[(signed __int16)a1.uObjectDescID].uSpeed,
                0);
            v13 = pDir->uYawAngle - 30;
        }
        a1.Create(
            v13, pDir->uPitchAngle,
            pObjectList->pObjects[(signed __int16)a1.uObjectDescID].uSpeed, 0);
    }
    return;
}

//----- (00404736) --------------------------------------------------------
void Actor::Explode(unsigned int uActorID) {
    SpriteObject a1;  // [sp+Ch] [bp-78h]@1

    a1.uType = SPRITE_600;
    // a1.uObjectDescID = GetObjDescId(a1.uType);  // no  objdesc for this sprtie - check
    a1.containing_item.Reset();
    a1.spell_id = 0;
    a1.spell_level = 0;
    a1.spell_skill = 0;
    a1.vPosition.x = pActors[uActorID].vPosition.x;
    a1.vPosition.y = pActors[uActorID].vPosition.y;
    a1.vPosition.z = pActors[uActorID].vPosition.z + (pActors[uActorID].uActorHeight * 0.75);
    a1.uFacing = 0;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID =
        pIndoor->GetSector(a1.vPosition.x, a1.vPosition.y, a1.vPosition.z);
    a1.uSpriteFrameID = 0;
    a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
    a1.spell_target_pid = 0;
    a1.field_60_distance_related_prolly_lod = 3;
    a1.field_61 = 4;
    a1.Create(0, 0, 0, 0);
    return;
}

//----- (004040E9) --------------------------------------------------------
// // Get direction vector from object1 to object2,
// // distance from object1 to object2 and Euler angles of the direction vector
// //
// //
// // object1 & object2 format :  objectType | (objectID << 3)
// //    objectType == 2 - SpriteObject
// //    objectType == 3 - Actor
// //    objectType == 4 - Party
// //    objectType == 5 - Decoration
// //
// // originally this function had following prototype:
// // struct DirectionInfo GetDirectionInfo(signed int object1, signed int
// object2, signed int a4)
// // but compiler converts functions returning structures by value in the such
// way
void Actor::GetDirectionInfo(unsigned int uObj1ID, unsigned int uObj2ID,
                             struct AIDirection *pOut, int a4) {
    signed int v4;    // eax@1
    signed int v5;    // ecx@1
    int v18;          // edx@15
    float v31;        // st7@45
    float v32;        // st6@45
    float v33;        // st7@45
    Vec3_int_ v37;    // [sp-10h] [bp-5Ch]@15
    AIDirection v41;  // [sp+14h] [bp-38h]@46
    float outy2;      // [sp+38h] [bp-14h]@33
    float outx2;      // [sp+3Ch] [bp-10h]@33
    int outz;         // [sp+40h] [bp-Ch]@6
    int outy;         // [sp+44h] [bp-8h]@6
    int outx;         // [sp+48h] [bp-4h]@6
    float a4a;        // [sp+58h] [bp+Ch]@45

    v4 = PID_ID(uObj1ID);
    // v6 = uObj2ID;
    v5 = PID_ID(uObj2ID);
    switch (PID_TYPE(uObj1ID)) {
        case OBJECT_Item: {
            outx = pSpriteObjects[v4].vPosition.x;
            outy = pSpriteObjects[v4].vPosition.y;
            outz = pSpriteObjects[v4].vPosition.z;
            break;
        }
        case OBJECT_Actor: {
            outx = pActors[v4].vPosition.x;
            outy = pActors[v4].vPosition.y;
            outz = pActors[v4].vPosition.z + (pActors[v4].uActorHeight * 0.75);
            break;
        }
        case OBJECT_Player: {
            if (!v4) {
                outx = pParty->vPosition.x;
                outy = pParty->vPosition.y;
                outz =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                break;
            }
            if (v4 == 4) {
                v18 = pParty->sRotationY - stru_5C6E00->uIntegerHalfPi;
                v37.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                v37.x = pParty->vPosition.x;
                v37.y = pParty->vPosition.y;
                Vec3_int_::Rotate(24, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 3) {
                v18 = pParty->sRotationY - stru_5C6E00->uIntegerHalfPi;
                v37.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                v37.x = pParty->vPosition.x;
                v37.y = pParty->vPosition.y;
                Vec3_int_::Rotate(8, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 2) {
                v37.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                v18 = stru_5C6E00->uIntegerHalfPi + pParty->sRotationY;
                v37.x = pParty->vPosition.x;
                v37.y = pParty->vPosition.y;
                Vec3_int_::Rotate(8, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 1) {
                v37.z =
                    pParty->vPosition.z + (signed int)pParty->uPartyHeight / 3;
                v18 = stru_5C6E00->uIntegerHalfPi + pParty->sRotationY;
                v37.x = pParty->vPosition.x;
                v37.y = pParty->vPosition.y;
                Vec3_int_::Rotate(24, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
        }
        case OBJECT_Decoration: {
            outx = pLevelDecorations[v4].vPosition.x;
            outy = pLevelDecorations[v4].vPosition.y;
            outz = pLevelDecorations[v4].vPosition.z;
            break;
        }
        default: {
            outz = 0;
            outy = 0;
            outx = 0;
            break;
        }
        case OBJECT_BModel: {
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                outx = (pIndoor->pFaces[v4].pBounding.x1 +
                        pIndoor->pFaces[v4].pBounding.x2) >>
                       1;
                outy = (pIndoor->pFaces[v4].pBounding.y1 +
                        pIndoor->pFaces[v4].pBounding.y2) >>
                       1;
                outz = (pIndoor->pFaces[v4].pBounding.z1 +
                        pIndoor->pFaces[v4].pBounding.z2) >>
                       1;
            }
            break;
        }
    }

    switch (PID_TYPE(uObj2ID)) {
        case OBJECT_Item: {
            outx2 = (float)pSpriteObjects[v5].vPosition.x;
            outy2 = (float)pSpriteObjects[v5].vPosition.y;
            a4 = pSpriteObjects[v5].vPosition.z;
            break;
        }
        case OBJECT_Actor: {
            outx2 = (float)pActors[v5].vPosition.x;
            outy2 = (float)pActors[v5].vPosition.y;
            a4 = pActors[v5].vPosition.z + (pActors[v5].uActorHeight * 0.75);
            break;
        }
        case OBJECT_Player: {
            outx2 = (float)pParty->vPosition.x;
            outy2 = (float)pParty->vPosition.y;
            if (!a4) a4 = pParty->sEyelevel;
            a4 = pParty->vPosition.z + a4;
            break;
        }
        case OBJECT_Decoration: {
            outx2 = (float)pLevelDecorations[v5].vPosition.x;
            outy2 = (float)pLevelDecorations[v5].vPosition.y;
            a4 = pLevelDecorations[v5].vPosition.z;
            break;
        }
        default: {
            outx2 = 0.0;
            outy2 = 0.0;
            a4 = 0;
            break;
        }
        case OBJECT_BModel: {
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                outx2 = (float)((pIndoor->pFaces[v5].pBounding.x1 +
                                 pIndoor->pFaces[v5].pBounding.x2) >>
                                1);
                outy2 = (float)((pIndoor->pFaces[v5].pBounding.y1 +
                                 pIndoor->pFaces[v5].pBounding.y2) >>
                                1);
                a4 = (pIndoor->pFaces[v5].pBounding.z1 +
                      pIndoor->pFaces[v5].pBounding.z2) >>
                     1;
            }
            break;
        }
    }

    v31 = (float)outx2 - (float)outx;
    v32 = (float)outy2 - (float)outy;
    a4a = (float)a4 - (float)outz;
    outx2 = v32 * v32;
    outy2 = v31 * v31;
    v33 = sqrt(a4a * a4a + outy2 + outx2);
    if (v33 <= 1.0) {
        pOut->vDirection.x = 65536;
        pOut->vDirection.y = 0;
        pOut->vDirection.z = 0;
        pOut->uDistance = 1;
        pOut->uDistanceXZ = 1;
        pOut->uYawAngle = 0;
        pOut->uPitchAngle = 0;
    } else {
        pOut->vDirection.x = (int32_t)(1.0 / v33 * v31 * 65536.0);
        pOut->vDirection.y = (int32_t)(1.0 / v33 * v32 * 65536.0);
        pOut->vDirection.z = (int32_t)(1.0 / v33 * a4a * 65536.0);
        pOut->uDistance = (uint)v33;
        pOut->uDistanceXZ = (uint)sqrt(outy2 + outx2);
        pOut->uYawAngle =
            stru_5C6E00->Atan2((signed __int64)v31, (signed __int64)v32);
        pOut->uPitchAngle =
            stru_5C6E00->Atan2(pOut->uDistanceXZ, (signed __int64)a4a);
    }
}

//----- (00404030) --------------------------------------------------------
void Actor::AI_FaceObject(unsigned int uActorID, unsigned int uObjID, int _48,
                          AIDirection *a4) {
    AIDirection *v7;  // eax@3
    AIDirection v1;   // eax@3
    AIDirection a3;   // [sp+8h] [bp-38h]@4

    if (rand() % 100 >= 5) {
        // v9 = &pActors[uActorID];
        if (!a4) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), uObjID, &v1,
                                    0);
            v7 = &v1;
        } else {
            v7 = a4;
        }
        pActors[uActorID].uYawAngle = v7->uYawAngle;
        pActors[uActorID].uCurrentActionTime = 0;
        pActors[uActorID].vVelocity.z = 0;
        pActors[uActorID].vVelocity.y = 0;
        pActors[uActorID].vVelocity.x = 0;
        pActors[uActorID].uPitchAngle = v7->uPitchAngle;
        pActors[uActorID].uCurrentActionLength = 256;
        pActors[uActorID].uAIState = Interacting;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Bored(uActorID, uObjID, a4);
    }
}

//----- (00403F58) --------------------------------------------------------
void Actor::AI_StandOrBored(unsigned int uActorID, signed int uObjID,
                            int uActionLength, AIDirection *a4) {
    if (rand() % 2)  // 0 or 1
        AI_Bored(uActorID, uObjID, a4);
    else
        AI_Stand(uActorID, uObjID, uActionLength, a4);
}

//----- (00403EB6) --------------------------------------------------------
void Actor::AI_Stand(unsigned int uActorID, unsigned int object_to_face_pid,
                     unsigned int uActionLength, AIDirection *a4) {
    assert(uActorID < uNumActors);
    // Actor* actor = &pActors[uActorID];

    AIDirection a3;
    if (!a4) {
        Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), object_to_face_pid,
                                &a3, 0);
        a4 = &a3;
    }

    pActors[uActorID].uAIState = Standing;
    if (!uActionLength)
        pActors[uActorID].uCurrentActionLength =
            rand() % 256 + 256;  // от 256 до 256 + 256
    else
        pActors[uActorID].uCurrentActionLength = uActionLength;
    pActors[uActorID].uCurrentActionTime = 0;
    pActors[uActorID].uYawAngle = a4->uYawAngle;
    pActors[uActorID].uPitchAngle = a4->uPitchAngle;
    pActors[uActorID].vVelocity.z = 0;
    pActors[uActorID].vVelocity.y = 0;
    pActors[uActorID].vVelocity.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403E61) --------------------------------------------------------
void Actor::StandAwhile(unsigned int uActorID) {
    pActors[uActorID].uCurrentActionLength = rand() % 128 + 128;
    pActors[uActorID].uCurrentActionTime = 0;
    pActors[uActorID].uAIState = Standing;
    pActors[uActorID].vVelocity.z = 0;
    pActors[uActorID].vVelocity.y = 0;
    pActors[uActorID].vVelocity.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403C6C) --------------------------------------------------------
void Actor::AI_MeleeAttack(unsigned int uActorID, signed int sTargetPid,
                           struct AIDirection *arg0) {
    int16_t v6;        // esi@6
    int16_t v7;        // edi@6
    signed int v8;     // eax@7
    Vec3_int_ v10;     // ST04_12@9
    AIDirection *v12;  // eax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@12
    AIDirection v20;   // [sp+28h] [bp-2Ch]@12
    int v23;           // [sp+4Ch] [bp-8h]@6
    unsigned int v25;  // [sp+5Ch] [bp+8h]@13

    assert(uActorID < uNumActors);

    if (pActors[uActorID].pMonsterInfo.uMovementType ==
            MONSTER_MOVEMENT_TYPE_STAIONARY &&
        pActors[uActorID].pMonsterInfo.uAIType == 1) {
        Actor::AI_Stand(uActorID, sTargetPid, 0, arg0);
        return;
    }

    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v8 = PID_ID(sTargetPid);
        v6 = pActors[v8].vPosition.x;
        v7 = pActors[v8].vPosition.y;
        v23 = (int)(pActors[v8].uActorHeight * 0.75 + pActors[v8].vPosition.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Player) {
        v6 = pParty->vPosition.x;
        v7 = pParty->vPosition.y;
        v23 = pParty->vPosition.z + pParty->sEyelevel;
    } else {
        Error("Should not get here");
        return;
    }

    v10.x = pActors[uActorID].vPosition.x;
    v10.y = pActors[uActorID].vPosition.y;
    v10.z = (int32_t)(pActors[uActorID].uActorHeight * 0.75 +
                      pActors[uActorID].vPosition.z);

    if (sub_407A1C((int)v6, (int)v7, v23, v10)) {
        if (arg0 != nullptr) {
            v12 = arg0;
        } else {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v12 = &a3;
        }
        pActors[uActorID].uYawAngle = (short)v12->uYawAngle;
        pActors[uActorID].uCurrentActionLength =
            pSpriteFrameTable
                ->pSpriteSFrames[pActors[uActorID].pSpriteIDs[ANIM_AtkMelee]]
                .uAnimLength *
            8;
        pActors[uActorID].uCurrentActionTime = 0;
        pActors[uActorID].uAIState = AttackingMelee;
        Actor::PlaySound(uActorID, 0);
        v25 = pMonsterStats->pInfos[pActors[uActorID].pMonsterInfo.uID]
                  .uRecoveryTime;
        if (pActors[uActorID].pActorBuffs[ACTOR_BUFF_SLOWED].Active()) v25 *= 2;
        if (!pParty->bTurnBasedModeOn) {
            pActors[uActorID].pMonsterInfo.uRecoveryTime = (int)(flt_6BE3A8_debug_recmod2 * v25 * 2.133333333333333);
        } else {
            pActors[uActorID].pMonsterInfo.uRecoveryTime = v25;
        }
        pActors[uActorID].vVelocity.z = 0;
        pActors[uActorID].vVelocity.y = 0;
        pActors[uActorID].vVelocity.x = 0;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, rand() % 2, 64, arg0);
    }
}

//----- (00438CF3) --------------------------------------------------------
void Actor::ApplyFineForKillingPeasant(unsigned int uActorID) {
    if (uLevelMapStatsID == 0 || !pActors[uActorID].IsPeasant()) return;

    if ((uLevelMapStatsID == 6 || uLevelMapStatsID == 7) &&
        pParty->IsPartyEvil())  // celeste and bracada
        return;

    if ((uLevelMapStatsID == 5 || uLevelMapStatsID == 8) &&
        pParty->IsPartyGood())  // the pit and deyja
        return;

    pParty->uFine += 100 * (pMapStats->pInfos[uLevelMapStatsID]._steal_perm +
                            pActors[uActorID].pMonsterInfo.uLevel +
                            pParty->GetPartyReputation());
    if (pParty->uFine < 0) pParty->uFine = 0;
    if (pParty->uFine > 4000000) pParty->uFine = 4000000;

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        if (pOutdoor->ddm.uReputation < 10000) pOutdoor->ddm.uReputation++;
    } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        if (pIndoor->dlv.uReputation < 10000) pIndoor->dlv.uReputation++;
    } else {
        assert(false);
    }

    if (pParty->uFine) {
        for (int i = 1; i <= 4; i++) {
            if (!_449B57_test_bit(pPlayers[i]->_achieved_awards_bits, PLAYER_GUILD_BITS__FINED))
                _449B7E_toggle_bit(pPlayers[i]->_achieved_awards_bits, PLAYER_GUILD_BITS__FINED, 1u);
        }
    }
}

//----- (0043AE80) --------------------------------------------------------
void Actor::AddBloodsplatOnDamageOverlay(unsigned int uActorID, int a2,
                                         signed int a3) {
    unsigned int v4;  // esi@1

    v4 = PID(OBJECT_Actor, uActorID);
    switch (a2) {
        case 1:
            if (a3)
                pOtherOverlayList->_4418B6(904, v4, 0,
                                           (int)(sub_43AE12(a3) * 65536.0), 0);
            return;
        case 2:
            if (a3)
                pOtherOverlayList->_4418B6(905, v4, 0,
                                           (int)(sub_43AE12(a3) * 65536.0), 0);
            return;
        case 3:
            if (a3)
                pOtherOverlayList->_4418B6(906, v4, 0,
                                           (int)(sub_43AE12(a3) * 65536.0), 0);
            return;
        case 4:
            if (a3)
                pOtherOverlayList->_4418B6(907, v4, 0,
                                           (int)(sub_43AE12(a3) * 65536.0), 0);
            return;
        case 5:
            pOtherOverlayList->_4418B6(901, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        case 6:
            pOtherOverlayList->_4418B6(902, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        case 7:
            pOtherOverlayList->_4418B6(903, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        case 8:
            pOtherOverlayList->_4418B6(900, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        case 9:
            pOtherOverlayList->_4418B6(909, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        case 10:
            pOtherOverlayList->_4418B6(908, v4, 0, PID(OBJECT_Actor, uActorID),
                                       0);
            return;
        default:
            return;
    }
    return;
}

//----- (0043B3E0) --------------------------------------------------------
int Actor::_43B3E0_CalcDamage(signed int dmgSource) {
    int v2;        // ebp@1
    int v3;               // eax@9
    int v4;        // edi@9
    int v5;               // esi@9
    unsigned __int16 v8;  // si@21
    int v9;               // edi@21
    int v10;       // eax@23
    int v11;              // [sp+10h] [bp-4h]@1

    v2 = 0;
    v11 = 0;

    switch (dmgSource) {
        case 0:
            if (this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
                v2 = this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].uPower;
            if (this->pActorBuffs[ACTOR_BUFF_HEROISM].Active() &&
                this->pActorBuffs[ACTOR_BUFF_HEROISM].uPower > v2)
                v2 = this->pActorBuffs[ACTOR_BUFF_HEROISM].uPower;
            if (this->pActorBuffs[ACTOR_BUFF_PAIN_HAMMERHANDS].Active())
                v2 += this->pActorBuffs[ACTOR_BUFF_PAIN_HAMMERHANDS].uPower;
            v3 = this->pMonsterInfo.uAttack1DamageDiceRolls;
            v4 = this->pMonsterInfo.uAttack1DamageDiceSides;
            v5 = this->pMonsterInfo.uAttack1DamageBonus;
            break;
        case 1:
            v3 = this->pMonsterInfo.uAttack2DamageDiceRolls;
            v4 = this->pMonsterInfo.uAttack2DamageDiceSides;
            v5 = this->pMonsterInfo.uAttack2DamageBonus;
            break;
        case 2:
            v8 = this->pMonsterInfo.uSpellSkillAndMastery1;
            v9 = this->pMonsterInfo.uSpell1ID;
            v10 = SkillToMastery(v8);
            return _43AFE3_calc_spell_damage(v9, v8 & 0x3F, v10, 0);
            break;
        case 3:
            v8 = this->pMonsterInfo.uSpellSkillAndMastery2;
            v9 = this->pMonsterInfo.uSpell2ID;
            v10 = SkillToMastery(v8);
            return _43AFE3_calc_spell_damage(v9, v8 & 0x3F, v10, 0);
            break;
        case 4:
            v3 = this->pMonsterInfo.uSpecialAbilityDamageDiceRolls;
            v4 = this->pMonsterInfo.uSpecialAbilityDamageDiceSides;
            v5 = this->pMonsterInfo.uSpecialAbilityDamageDiceBonus;
        default:
            return 0;
    }
    for (int i = 0; i < v3; i++) v11 += rand() % v4 + 1;
    return v11 + v5 + v2;
}

//----- (00438B9B) --------------------------------------------------------
bool Actor::IsPeasant() {
    unsigned int InHostile_Id;  // eax@1

    InHostile_Id = this->uAlly;
    if (!this->uAlly) InHostile_Id = (this->pMonsterInfo.uID - 1) / 3 + 1;
    return (signed int)InHostile_Id >= 39 &&
               (signed int)InHostile_Id <= 44  // Dwarfs peasants
           || (signed int)InHostile_Id >= 45 &&
                  (signed int)InHostile_Id <= 50  // Elves peasants
           || (signed int)InHostile_Id >= 51 &&
                  (signed int)InHostile_Id <= 62  // Humans peasants
           || (signed int)InHostile_Id >= 78 &&
                  (signed int)InHostile_Id <= 83;  // Goblins peasants
}

//----- (0042EBEE) --------------------------------------------------------
void Actor::StealFrom(unsigned int uActorID) {
    Player *pPlayer;     // edi@1
    int v4;              // ebx@2
    unsigned int v5;     // eax@2
    DDM_DLV_Header *v6;  // esi@4
    int v8;              // [sp+8h] [bp-4h]@6

    pPlayer = &pParty->pPlayers[uActiveCharacter - 1];
    if (pPlayer->CanAct()) {
        CastSpellInfoHelpers::_427D48();
        v4 = 0;
        v5 = pMapStats->GetMapInfo(pCurrentMapName);
        if (v5) v4 = pMapStats->pInfos[v5]._steal_perm;
        v6 = &pOutdoor->ddm;
        if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v6 = &pIndoor->dlv;
        pPlayer->StealFromActor(uActorID, v4, v6->uReputation++);
        v8 = pPlayer->GetAttackRecoveryTime(0);
        if (v8 < 30) v8 = 30;
        if (!pParty->bTurnBasedModeOn)
            pPlayer->SetRecoveryTime(
                (int)(flt_6BE3A4_debug_recmod1 * v8 * 2.133333333333333));
        pTurnEngine->ApplyPlayerAction();
    }
    return;
}

//----- (00403A60) --------------------------------------------------------
void Actor::AI_SpellAttack2(unsigned int uActorID, signed int edx0,
                            AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3_int_ v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    __int16 v13;         // ax@10
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v18;     // [sp+28h] [bp-2Ch]@9
    int v19;             // [sp+44h] [bp-10h]@6
    signed int a2;       // [sp+48h] [bp-Ch]@1
    int v21;             // [sp+4Ch] [bp-8h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    a2 = edx0;
    if (PID_TYPE(edx0) == OBJECT_Actor) {
        v6 = PID_ID(edx0);
        v4 = pActors[v6].vPosition.x;
        v5 = pActors[v6].vPosition.y;
        v21 = (int)(pActors[v6].uActorHeight * 0.75 + pActors[v6].vPosition.z);
    } else if (PID_TYPE(edx0) == OBJECT_Player) {
        v4 = pParty->vPosition.x;
        v5 = pParty->vPosition.y;
        v21 = pParty->vPosition.z + pParty->sEyelevel;
    } else {
        Error("Should not get here");
        return;
    }
    v19 = v3->uActorHeight;
    v7.z = v3->vPosition.z + (v19 * 0.75);
    v7.y = v3->vPosition.y;
    v7.x = v3->vPosition.x;
    if (sub_407A1C(v4, v5, v21, v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), a2, &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->uYawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->pSpriteIDs[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->uCurrentActionLength = 8 * v13;
        v3->uCurrentActionTime = 0;
        v3->uAIState = AttackingRanged4;
        Actor::PlaySound(uActorID, 0);
        pDira = pMonsterStats->pInfos[v3->pMonsterInfo.uID].uRecoveryTime;
        if (v3->pActorBuffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->pMonsterInfo.uRecoveryTime = pDira;
        } else {
            v3->pMonsterInfo.uRecoveryTime = v3->uCurrentActionLength + (int)(flt_6BE3A8_debug_recmod2 * pDira * 2.133333333333333);
        }
        v3->vVelocity.z = 0;
        v3->vVelocity.y = 0;
        v3->vVelocity.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->pMonsterInfo.uSpell2ID)) {
            v3->uCurrentActionLength = 64;
            v3->uCurrentActionTime = 0;
            v3->uAIState = Fidgeting;
            v3->UpdateAnimation();
            v3->uAIState = AttackingRanged4;
        } else {
            v3->UpdateAnimation();
        }
    } else {
        Actor::AI_Pursue1(uActorID, a2, uActorID, 64, pDir);
    }
}

//----- (00403854) --------------------------------------------------------
void Actor::AI_SpellAttack1(unsigned int uActorID, signed int sTargetPid,
                            AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3_int_ v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    __int16 v13;         // ax@10
    signed int v16;      // ecx@17
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v18;     // [sp+28h] [bp-2Ch]@9
    int v19;             // [sp+44h] [bp-10h]@6
    int v21;             // [sp+4Ch] [bp-8h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        v4 = pActors[v6].vPosition.x;
        v5 = pActors[v6].vPosition.y;
        v21 = (int)(pActors[v6].uActorHeight * 0.75 + pActors[v6].vPosition.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Player) {
        v4 = pParty->vPosition.x;
        v5 = pParty->vPosition.y;
        v21 = pParty->vPosition.z + pParty->sEyelevel;
    } else {
        Error("Should not get here");
        return;
    }
    v19 = v3->uActorHeight;
    v7.z = v3->vPosition.z + (v19 * 0.75);
    v7.y = v3->vPosition.y;
    v7.x = v3->vPosition.x;
    if (sub_407A1C(v4, v5, v21, v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->uYawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->pSpriteIDs[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->uCurrentActionLength = 8 * v13;
        v3->uCurrentActionTime = 0;
        v3->uAIState = AttackingRanged3;
        Actor::PlaySound(uActorID, 0);
        pDira = pMonsterStats->pInfos[v3->pMonsterInfo.uID].uRecoveryTime;
        if (v3->pActorBuffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->pMonsterInfo.uRecoveryTime = pDira;
        } else {
            v3->pMonsterInfo.uRecoveryTime = v3->uCurrentActionLength + (int)(flt_6BE3A8_debug_recmod2 * pDira * 2.133333333333333);
        }
        v16 = v3->pMonsterInfo.uSpell1ID;
        v3->vVelocity.z = 0;
        v3->vVelocity.y = 0;
        v3->vVelocity.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->pMonsterInfo.uSpell1ID)) {
            v3->uCurrentActionLength = 64;
            v3->uCurrentActionTime = 0;
            v3->uAIState = Fidgeting;
            v3->UpdateAnimation();
            v3->uAIState = AttackingRanged3;
        } else {
            v3->UpdateAnimation();
        }
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64, pDir);
    }
}

//----- (0040368B) --------------------------------------------------------
void Actor::AI_MissileAttack2(unsigned int uActorID, signed int sTargetPid,
                              AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3_int_ v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    __int16 v13;         // ax@10
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v17;     // [sp+28h] [bp-2Ch]@9
    int v18;             // [sp+44h] [bp-10h]@6
    int v20;             // [sp+4Ch] [bp-8h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        v4 = pActors[v6].vPosition.x;
        v5 = pActors[v6].vPosition.y;
        v20 = (int)(pActors[v6].uActorHeight * 0.75 + pActors[v6].vPosition.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Player) {
        v4 = pParty->vPosition.x;
        v5 = pParty->vPosition.y;
        v20 = pParty->vPosition.z + pParty->sEyelevel;
    } else {
        Error("Should not get here");
        return;
    }
    v18 = v3->uActorHeight;
    v7.z = v3->vPosition.z + (v18 * 0.75);
    v7.y = v3->vPosition.y;
    v7.x = v3->vPosition.x;
    if (sub_407A1C(v4, v5, v20, v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->uYawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->pSpriteIDs[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->uCurrentActionLength = 8 * v13;
        v3->uCurrentActionTime = 0;
        v3->uAIState = AttackingRanged2;
        Actor::PlaySound(uActorID, 0);
        pDira = pMonsterStats->pInfos[v3->pMonsterInfo.uID].uRecoveryTime;
        if (v3->pActorBuffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (!pParty->bTurnBasedModeOn) {
            v3->pMonsterInfo.uRecoveryTime = (int)(flt_6BE3A8_debug_recmod2 * pDira * 2.133333333333333);
        } else {
            v3->pMonsterInfo.uRecoveryTime = pDira;
        }
        v3->vVelocity.z = 0;
        v3->vVelocity.y = 0;
        v3->vVelocity.x = 0;
        v3->UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64, pDir);
    }
}

//----- (00403476) --------------------------------------------------------
void Actor::AI_MissileAttack1(unsigned int uActorID, signed int sTargetPid,
                              AIDirection *pDir) {
    Actor *v3;         // ebx@1
    int v4;            // esi@3
    int v5;            // edi@3
    signed int v6;     // eax@4
    Vec3_int_ v7;      // ST04_12@6
    AIDirection *v10;  // eax@9
    __int16 v14;       // ax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@10
    AIDirection v18;   // [sp+28h] [bp-2Ch]@10
    int v19;           // [sp+44h] [bp-10h]@6
    // signed int a2; // [sp+48h] [bp-Ch]@1
    int v22;             // [sp+50h] [bp-4h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@11

    v3 = &pActors[uActorID];
    // a2 = edx0;
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        v4 = pActors[v6].vPosition.x;
        v5 = pActors[v6].vPosition.y;
        v22 = (int)(pActors[v6].uActorHeight * 0.75 + pActors[v6].vPosition.z);
    } else {
        if (PID_TYPE(sTargetPid) == OBJECT_Player) {
            v4 = pParty->vPosition.x;
            v5 = pParty->vPosition.y;
            v22 = pParty->vPosition.z + pParty->sEyelevel;
        } else {
            v4 = (int)pDir;
            v5 = (int)pDir;
        }
    }
    v19 = v3->uActorHeight;
    v7.z = v3->vPosition.z + (v19 * 0.75);
    v7.y = v3->vPosition.y;
    v7.x = v3->vPosition.x;
    if (sub_407A1C(v4, v5, v22, v7) ||
        sub_407A1C(v7.x, v7.y, v7.z, Vec3_int_(v4, v5, v22))) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v10 = &a3;
        } else {
            v10 = pDir;
        }
        v3->uYawAngle = (short)v10->uYawAngle;
        v14 = pSpriteFrameTable->pSpriteSFrames[v3->pSpriteIDs[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->uCurrentActionLength = 8 * v14;
        v3->uCurrentActionTime = 0;
        v3->uAIState = AttackingRanged1;
        Actor::PlaySound(uActorID, 0);
        pDira = pMonsterStats->pInfos[v3->pMonsterInfo.uID].uRecoveryTime;
        if (v3->pActorBuffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->pMonsterInfo.uRecoveryTime = pDira;
        } else {
            v3->pMonsterInfo.uRecoveryTime = v3->uCurrentActionLength - (int)(flt_6BE3A8_debug_recmod2 * pDira * -2.133333333333333);
        }
        v3->vVelocity.z = 0;
        v3->vVelocity.y = 0;
        v3->vVelocity.x = 0;
        v3->UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64, pDir);
    }
}

//----- (004032B2) --------------------------------------------------------
void Actor::AI_RandomMove(unsigned int uActor_id, unsigned int uTarget_id,
                          int radius, int uActionLength) {
    int x;                                             // ebx@1
    int absy;                                          // eax@1
    unsigned int v9;                                   // ebx@11
    int v10;                                           // ebx@13
    AIDirection doNotInitializeBecauseShouldBeRandom;  // [sp+Ch] [bp-30h]@7
    int y;                                             // [sp+30h] [bp-Ch]@1
    int absx;                                          // [sp+38h] [bp-4h]@1

    x = pActors[uActor_id].vInitialPosition.x - pActors[uActor_id].vPosition.x;
    y = pActors[uActor_id].vInitialPosition.y - pActors[uActor_id].vPosition.y;
    absx = abs(x);
    absy = abs(y);
    if (absx <= absy)
        absx = absy + (absx / 2);
    else
        absx = absx + absy / 2;
    if (MonsterStats::BelongsToSupertype(pActors[uActor_id].pMonsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_StandOrBored(uActor_id, OBJECT_Player, uActionLength,
                               &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    if (pActors[uActor_id].pMonsterInfo.uMovementType ==
            MONSTER_MOVEMENT_TYPE_GLOBAL &&
        absx < 128) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256,
                        &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    absx += ((rand() & 0xF) * radius) / 16;
    v9 = (stru_5C6E00->uIntegerDoublePi - 1) & stru_5C6E00->Atan2(x, y);
    if (rand() % 100 < 25) {
        Actor::StandAwhile(uActor_id);
        return;
    }
    v10 = v9 + rand() % 256 - 128;
    if (abs(v10 - pActors[uActor_id].uYawAngle) > 256 &&
        !(pActors[uActor_id].uAttributes & ACTOR_ANIMATION)) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256,
                        &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    pActors[uActor_id].uYawAngle = v10;
    if (pActors[uActor_id].uMovementSpeed)
        pActors[uActor_id].uCurrentActionLength =
            32 * absx / pActors[uActor_id].uMovementSpeed;
    else
        pActors[uActor_id].uCurrentActionLength = 0;
    pActors[uActor_id].uCurrentActionTime = 0;
    pActors[uActor_id].uAIState = Tethered;
    if (rand() % 100 < 2) Actor::PlaySound(uActor_id, 3);
    pActors[uActor_id].UpdateAnimation();
}

//----- (004031C1) --------------------------------------------------------
char Actor::_4031C1_update_job_never_gets_called(
    unsigned int uActorID, signed int a2,
    int a3) {  // attempted to implement something like jobs for actors, but
               // apparently was never finished
    return 0;
    /*unsigned int v3; // edi@1
    Actor *v4; // esi@1
    ActorJob *v5; // eax@1
    signed int v6; // edx@2
    ActorJob *v7; // eax@2
    signed int v8; // edi@2
    ActorJob *v9; // ecx@2
    __int16 v10; // cx@15
    signed int v12; // [sp+8h] [bp-4h]@1

    v3 = uActorID;
    v12 = a2;
    v4 = &pActors[uActorID];
    v5 = (ActorJob *)pActors[uActorID].CanAct();
    if ( v5 )
    {
      v6 = 65535;
      v7 = &v4->pScheduledJobs[v3];
      v8 = 7;
      v9 = &v7[7];//(char *)&v7[7].uHour;
          while ( !(v9->uAttributes & 1) || v9->uHour > v12 )
      {
        --v8;
        --v9;
        if ( v8 < 0 )
          break;
      }
          if( v8 >= 0 )
                  v6 = v8;
      if ( !v8 && v6 == 65535 )
        v6 = 7;
      v5 = &v7[v6];
      if ( v4->vInitialPosition.x != v5->vPos.x
        || v4->vInitialPosition.y != v5->vPos.y
        || v4->vInitialPosition.z != v5->vPos.z
        || v4->pMonsterInfo.uMovementType != v5->uAction )
      {
        v4->vInitialPosition.x = v5->vPos.x;
        v4->vInitialPosition.y = v5->vPos.y;
        v10 = v5->vPos.z;
        v4->vInitialPosition.z = v10;
        LOBYTE(v5) = v5->uAction;
        v4->pMonsterInfo.uMovementType = MONSTER_MOVEMENT_TYPE_STAIONARY;
        if ( a3 == 1 )
        {
          v4->vPosition.x = v4->vInitialPosition.x;
          v4->vPosition.y = v4->vInitialPosition.y;
          LOBYTE(v5) = v10;
          v4->vPosition.z = v10;
        }
      }
    }
    return (char)v5;*/
}

//----- (004030AD) --------------------------------------------------------
void Actor::AI_Stun(unsigned int uActorID, signed int edx0,
                    int stunRegardlessOfState) {
    __int16 v7;      // ax@16
    AIDirection a3;  // [sp+Ch] [bp-40h]@16

    if (pActors[uActorID].uAIState == Fleeing)
        pActors[uActorID].uAttributes |= ACTOR_FLEEING;
    if (pActors[uActorID].pMonsterInfo.uHostilityType != 4) {
        pActors[uActorID].uAttributes &= 0xFFFFFFFB;  // ~0x4
        pActors[uActorID].pMonsterInfo.uHostilityType =
            MonsterInfo::Hostility_Long;
    }
    if (pActors[uActorID].pActorBuffs[ACTOR_BUFF_CHARM].Active())
        pActors[uActorID].pActorBuffs[ACTOR_BUFF_CHARM].Reset();
    if (pActors[uActorID].pActorBuffs[ACTOR_BUFF_AFRAID].Active())
        pActors[uActorID].pActorBuffs[ACTOR_BUFF_AFRAID].Reset();
    if (stunRegardlessOfState ||
        (pActors[uActorID].uAIState != Stunned &&
         pActors[uActorID].uAIState != AttackingRanged1 &&
         pActors[uActorID].uAIState != AttackingRanged2 &&
         pActors[uActorID].uAIState != AttackingRanged3 &&
         pActors[uActorID].uAIState != AttackingRanged4 &&
         pActors[uActorID].uAIState != AttackingMelee)) {
        Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), edx0, &a3, 0);
        // v10 = &a3;
        pActors[uActorID].uYawAngle = (short)a3.uYawAngle;
        v7 = pSpriteFrameTable
                 ->pSpriteSFrames[pActors[uActorID].pSpriteIDs[ANIM_GotHit]]
                 .uAnimLength;
        pActors[uActorID].uCurrentActionTime = 0;
        pActors[uActorID].uAIState = Stunned;
        pActors[uActorID].uCurrentActionLength = 8 * v7;
        Actor::PlaySound(uActorID, 2);
        pActors[uActorID].UpdateAnimation();
    }
}

//----- (00402F87) --------------------------------------------------------
void Actor::AI_Bored(unsigned int uActorID, unsigned int uObjID,
                     AIDirection *a4) {
    unsigned int v7;  // eax@3
    unsigned int v9;  // eax@3

    Actor *actor = &pActors[uActorID];

    AIDirection a3;  // [sp+Ch] [bp-5Ch]@2
    if (!a4) {
        Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), uObjID, &a3, 0);
        a4 = &a3;
    }

    actor->uCurrentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[actor->pSpriteIDs[ANIM_Bored]]
                .uAnimLength;

    v7 = stru_5C6E00->Atan2(actor->vPosition.x - pIndoorCameraD3D->vPartyPos.x,
                            actor->vPosition.y - pIndoorCameraD3D->vPartyPos.y);
    v9 = stru_5C6E00->uIntegerPi + actor->uYawAngle +
         ((signed int)stru_5C6E00->uIntegerPi >> 3) - v7;

    if (v9 & 0x700) {  // turned away - just stand
        Actor::AI_Stand(uActorID, uObjID, actor->uCurrentActionLength, a4);
    } else {  // facing player - play bored anim
        actor->uAIState = Fidgeting;
        actor->uCurrentActionTime = 0;
        actor->uYawAngle = a4->uYawAngle;
        actor->vVelocity.z = 0;
        actor->vVelocity.y = 0;
        actor->vVelocity.x = 0;
        if (rand() % 100 < 5) Actor::PlaySound(uActorID, 3);
        actor->UpdateAnimation();
    }
}

//----- (00402F27) --------------------------------------------------------
void Actor::Resurrect(unsigned int uActorID) {
    Actor *pActor;  // esi@1

    pActor = &pActors[uActorID];
    pActor->uCurrentActionTime = 0;
    pActor->uAIState = Resurrected;
    pActor->uCurrentActionAnimation = ANIM_Dying;
    pActor->uCurrentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[pActor->pSpriteIDs[ANIM_Dying]]
                .uAnimLength;
    pActor->sCurrentHP = (short)pActor->pMonsterInfo.uHP;
    Actor::PlaySound(uActorID, 1);
    pActor->UpdateAnimation();
}

//----- (00402D6E) --------------------------------------------------------
void Actor::Die(unsigned int uActorID) {
    Actor *actor = &pActors[uActorID];

    actor->uCurrentActionTime = 0;
    actor->uAIState = Dying;
    actor->uCurrentActionAnimation = ANIM_Dying;
    actor->sCurrentHP = 0;
    actor->uCurrentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[actor->pSpriteIDs[ANIM_Dying]]
                .uAnimLength;
    actor->pActorBuffs[ACTOR_BUFF_PARALYZED].Reset();
    actor->pActorBuffs[ACTOR_BUFF_STONED].Reset();
    Actor::PlaySound(uActorID, 1);
    actor->UpdateAnimation();

    for (uint i = 0; i < 5; ++i)
        if (pParty->monster_id_for_hunting[i] == actor->pMonsterInfo.uID)
            pParty->monster_for_hunting_killed[i] = true;

    for (uint i = 0; i < 22; ++i) actor->pActorBuffs[i].Reset();

    ItemGen drop;
    drop.Reset();
    switch (actor->pMonsterInfo.uID) {
        case MONSTER_HARPY_1:
        case MONSTER_HARPY_2:
        case MONSTER_HARPY_3:
            drop.uItemID = ITEM_HARPY_FEATHER;
            break;

        case MONSTER_OOZE_1:
        case MONSTER_OOZE_2:
        case MONSTER_OOZE_3:
            drop.uItemID = ITEM_OOZE_ENDOPLASM_VIAL;
            break;

        case MONSTER_TROLL_1:
        case MONSTER_TROLL_2:
        case MONSTER_TROLL_3:
            drop.uItemID = ITEM_TROLL_BLOOD;
            break;

        case MONSTER_DEVIL_1:
        case MONSTER_DEVIL_2:
        case MONSTER_DEVIL_3:
            drop.uItemID = ITEM_DEVIL_ICHOR;
            break;

        case MONSTER_DRAGON_1:
        case MONSTER_DRAGON_2:
        case MONSTER_DRAGON_3:
            drop.uItemID = ITEM_DRAGON_EYE;
            break;
    }

    if (rand() % 100 < 20 && drop.uItemID != 0) {
        SpriteObject::sub_42F7EB_DropItemAt(
            (SPRITE_OBJECT_TYPE)pItemsTable->pItems[drop.uItemID].uSpriteID,
            actor->vPosition.x, actor->vPosition.y, actor->vPosition.z + 16,
            rand() % 200 + 200, 1, 1, 0, &drop);
    }

    if (actor->pMonsterInfo.uSpecialAbilityType ==
        MONSTER_SPECIAL_ABILITY_EXPLODE)
        Actor::Explode(uActorID);
}

//----- (00402CED) --------------------------------------------------------
void Actor::PlaySound(unsigned int uActorID, unsigned int uSoundID) {
    SoundID sound_sample_id =
        (SoundID)pActors[uActorID].pSoundSampleIDs[uSoundID];
    if (sound_sample_id) {
        if (!pActors[uActorID].pActorBuffs[ACTOR_BUFF_SHRINK].Active()) {
            pAudioPlayer->PlaySound(sound_sample_id,
                PID(OBJECT_Actor, uActorID), 0, -1, 0,
                0);
        } else {
            switch (pActors[uActorID].pActorBuffs[ACTOR_BUFF_SHRINK].uPower) {
                case 1:
                    pAudioPlayer->PlaySound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0, 0);
                    break;
                case 2:
                    pAudioPlayer->PlaySound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0, 0);
                    break;
                case 3:
                case 4:
                    pAudioPlayer->PlaySound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0, 0);
                    break;
                default:
                    pAudioPlayer->PlaySound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, -1, 0, 0);
                    break;
            }
        }
    }
}

//----- (00402AD7) --------------------------------------------------------
void Actor::AI_Pursue1(unsigned int uActorID, unsigned int a2, signed int arg0,
                       signed int uActionLength, AIDirection *pDir) {
    int v6;            // eax@1
    Actor *v7;         // ebx@1
    unsigned int v8;   // ecx@1
    AIDirection *v10;  // esi@6
    AIDirection a3;    // [sp+Ch] [bp-5Ch]@7
    unsigned int v18;  // [sp+64h] [bp-4h]@1

    v6 = 0;
    v7 = &pActors[uActorID];
    v8 = PID(OBJECT_Actor, uActorID);
    if (v7->pMonsterInfo.uFlying != 0 &&
        !pParty->bFlying) {  // TODO(_): Does v6 have a point?
        if (v7->pMonsterInfo.uMissleAttack1Type)
            v6 = v7->uActorRadius + 512;
        else
            v6 = pParty->uPartyHeight;
    }

    if (pDir == nullptr) {
        Actor::GetDirectionInfo(v8, a2, &a3, v6);
        v10 = &a3;
    } else {
        v10 = pDir;
    }
    if (MonsterStats::BelongsToSupertype(v7->pMonsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_StandOrBored(uActorID, 4, uActionLength, v10);
        return;
    }
    if (v10->uDistance < 307.2) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_Stand(uActorID, a2, uActionLength, v10);
        return;
    }
    if (v7->uMovementSpeed == 0) {
        Actor::AI_Stand(uActorID, a2, uActionLength, v10);
        return;
    }
    if (arg0 % 2)
        v18 = -16;
    else
        v18 = 16;

    v7->uYawAngle = stru_5C6E00->Atan2(
        pParty->vPosition.x +
            (int)fixpoint_mul(stru_5C6E00->Cos(v18 + stru_5C6E00->uIntegerPi +
                                               v10->uYawAngle),
                              v10->uDistanceXZ) -
            v7->vPosition.x,
        pParty->vPosition.y +
            (int)fixpoint_mul(stru_5C6E00->Sin(v18 + stru_5C6E00->uIntegerPi +
                                               v10->uYawAngle),
                              v10->uDistanceXZ) -
            v7->vPosition.y);
    if (uActionLength)
        v7->uCurrentActionLength = uActionLength;
    else
        v7->uCurrentActionLength = 128;
    v7->uPitchAngle = (short)v10->uPitchAngle;
    v7->uAIState = Pursuing;
    v7->UpdateAnimation();
}

//----- (00402968) --------------------------------------------------------
void Actor::AI_Flee(unsigned int uActorID, signed int sTargetPid,
                    int uActionLength, AIDirection *a4) {
    Actor *v5;         // ebx@1
    int v7;            // ecx@2
    AIDirection v10;   // [sp+8h] [bp-7Ch]@4
    AIDirection a3;    // [sp+24h] [bp-60h]@3
    AIDirection *v13;  // [sp+5Ch] [bp-28h]@4

    v5 = &pActors[uActorID];
    if (v5->CanAct()) {
        v7 = PID(OBJECT_Actor, uActorID);
        if (!a4) {
            Actor::GetDirectionInfo(v7, sTargetPid, &a3,
                                    v5->pMonsterInfo.uFlying);
            a4 = &a3;
        }
        Actor::GetDirectionInfo(v7, 4u, &v10, 0);
        v13 = &v10;
        if (MonsterStats::BelongsToSupertype(v5->pMonsterInfo.uID,
                                             MONSTER_SUPERTYPE_TREANT) ||
            PID_TYPE(sTargetPid) == OBJECT_Actor && v13->uDistance < 307.2) {
            if (!uActionLength) uActionLength = 256;
            Actor::AI_StandOrBored(uActorID, 4, uActionLength, v13);
        } else {
            if (v5->uMovementSpeed)
                v5->uCurrentActionLength =
                    (signed int)(a4->uDistanceXZ << 7) / v5->uMovementSpeed;
            else
                v5->uCurrentActionLength = 0;
            if (v5->uCurrentActionLength > 256) v5->uCurrentActionLength = 256;
            v5->uYawAngle =
                (short)stru_5C6E00->uIntegerHalfPi + (short)a4->uYawAngle;
            v5->uYawAngle =
                (short)stru_5C6E00->uDoublePiMask &
                (v5->uYawAngle + rand() % (signed int)stru_5C6E00->uIntegerPi);
            v5->uCurrentActionTime = 0;
            v5->uPitchAngle = (short)a4->uPitchAngle;
            v5->uAIState = Fleeing;
            v5->UpdateAnimation();
        }
    }
}

//----- (0040281C) --------------------------------------------------------
void Actor::AI_Pursue2(unsigned int uActorID, unsigned int a2,
                       signed int uActionLength, AIDirection *pDir, int a5) {
    int v6;                // eax@1
    Actor *v7;             // ebx@1
    unsigned int v8;       // ecx@1
    AIDirection *v10;      // esi@7
    signed __int16 v13;    // cx@19
    unsigned __int16 v14;  // ax@25
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection v18;       // [sp+28h] [bp-24h]@8

    v6 = 0;
    v7 = &pActors[uActorID];
    v8 = PID(OBJECT_Actor, uActorID);
    if (v7->pMonsterInfo.uFlying != 0 && !pParty->bFlying) {
        if (v7->pMonsterInfo.uMissleAttack1Type &&
            uCurrentlyLoadedLevelType == LEVEL_Outdoor)
            v6 = v7->uActorRadius + 512;
        else
            v6 = pParty->uPartyHeight;
    }
    v10 = pDir;
    if (!pDir) {
        Actor::GetDirectionInfo(v8, a2, &a3, v6);
        v10 = &a3;
    }
    if (MonsterStats::BelongsToSupertype(v7->pMonsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_StandOrBored(uActorID, 4, uActionLength, v10);
        return;
    }
    if ((signed int)v10->uDistance < a5) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_StandOrBored(uActorID, a2, uActionLength, v10);
        return;
    }
    if (uActionLength) {
        v7->uCurrentActionLength = uActionLength;
    } else {
        v13 = v7->uMovementSpeed;
        if (v13)
            v7->uCurrentActionLength =
                (signed int)(v10->uDistanceXZ << 7) / v13;
        else
            v7->uCurrentActionLength = 0;
        if (v7->uCurrentActionLength > 32) v7->uCurrentActionLength = 32;
    }
    v7->uYawAngle = (short)v10->uYawAngle;
    v14 = (short)v10->uPitchAngle;
    v7->uCurrentActionTime = 0;
    v7->uPitchAngle = v14;
    v7->uAIState = Pursuing;
    v7->UpdateAnimation();
}

//----- (00402686) --------------------------------------------------------
void Actor::AI_Pursue3(unsigned int uActorID, unsigned int a2,
                       signed int uActionLength, AIDirection *a4) {
    int v5;                // eax@1
    Actor *v6;             // ebx@1
    int v7;                // ecx@1
    signed __int16 v12;    // cx@19
    __int16 v14;           // ax@25
    unsigned __int16 v16;  // ax@28
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection *v20;      // [sp+28h] [bp-24h]@8

    v5 = 0;
    v6 = &pActors[uActorID];
    v7 = PID(OBJECT_Actor, uActorID);
    if (v6->pMonsterInfo.uFlying != 0 && !pParty->bFlying) {
        if (v6->pMonsterInfo.uMissleAttack1Type &&
            uCurrentlyLoadedLevelType == LEVEL_Outdoor)
            v5 = v6->uActorRadius + 512;
        else
            v5 = pParty->uPartyHeight;
    }
    if (!a4) {
        Actor::GetDirectionInfo(v7, a2, &a3, v5);
        v20 = &a3;
    }
    if (MonsterStats::BelongsToSupertype(v6->pMonsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        return Actor::AI_StandOrBored(uActorID, 4, uActionLength, a4);
    }
    if (a4->uDistance < 307.2) {
        if (!uActionLength) uActionLength = 256;
        return Actor::AI_StandOrBored(uActorID, a2, uActionLength, a4);
    }
    if (uActionLength) {
        v6->uCurrentActionLength = uActionLength + rand() % uActionLength;
    } else {
        v12 = v6->uMovementSpeed;
        if (v12)
            v6->uCurrentActionLength = (signed int)(a4->uDistanceXZ << 7) / v12;
        else
            v6->uCurrentActionLength = 0;
        if (v6->uCurrentActionLength > 128) v6->uCurrentActionLength = 128;
    }
    v14 = (short)a4->uYawAngle;
    if (rand() % 2)
        v14 += 256;
    else
        v14 -= 256;
    v6->uYawAngle = v14;
    v16 = (short)a4->uPitchAngle;
    v6->uCurrentActionTime = 0;
    v6->uPitchAngle = v16;
    v6->uAIState = Pursuing;
    if (rand() % 100 < 2) Actor::PlaySound(uActorID, 2);
    v6->UpdateAnimation();
}

//----- (00401221) --------------------------------------------------------
void Actor::_SelectTarget(unsigned int uActorID, int *a2,
                          bool can_target_party) {
    int v5;                     // ecx@1
    signed int v10;             // eax@13
    uint v11;                   // ebx@16
    uint v12;                   // eax@16
    signed int v14;             // eax@31
    uint v15;                   // edi@43
    uint v16;                   // ebx@45
    uint v17;                   // eax@45
    signed int closestId;       // [sp+14h] [bp-1Ch]@1
    uint v23;                   // [sp+1Ch] [bp-14h]@16
    unsigned int lowestRadius;  // [sp+24h] [bp-Ch]@1
    uint v27;                   // [sp+2Ch] [bp-4h]@16
    uint v28;                   // [sp+2Ch] [bp-4h]@45

    lowestRadius = UINT_MAX;
    v5 = 0;
    *a2 = 0;
    closestId = 0;
    assert(uActorID < uNumActors);
    Actor *thisActor = &pActors[uActorID];

    for (uint i = 0; i < uNumActors; ++i) {
        Actor *actor = &pActors[i];
        if (actor->uAIState == Dead || actor->uAIState == Dying ||
            actor->uAIState == Removed || actor->uAIState == Summoned ||
            actor->uAIState == Disabled || uActorID == i)
            continue;

        if (thisActor->uLastCharacterIDToHit == 0 ||
            PID(OBJECT_Actor, v5) != thisActor->uLastCharacterIDToHit) {
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == 0) continue;
        } else if (thisActor->IsNotAlive()) {
            thisActor->uLastCharacterIDToHit = 0;
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == 0) continue;
        } else {
            if ((actor->uGroup != 0 || thisActor->uGroup != 0) &&
                actor->uGroup == thisActor->uGroup)
                continue;
            v10 = 4;
        }
        if (thisActor->pMonsterInfo.uHostilityType)
            v10 = pMonsterStats->pInfos[thisActor->pMonsterInfo.uID]
                      .uHostilityType;
        v11 = _4DF380_hostilityRanges[v10];
        v23 = abs(thisActor->vPosition.x - actor->vPosition.x);
        v27 = abs(thisActor->vPosition.y - actor->vPosition.y);
        v12 = abs(thisActor->vPosition.z - actor->vPosition.z);
        if (v23 <= v11 && v27 <= v11 && v12 <= v11 &&
            sub_4070EF_prolly_detect_player(PID(OBJECT_Actor, i),
                                            PID(OBJECT_Actor, uActorID)) &&
            v23 * v23 + v27 * v27 + v12 * v12 < lowestRadius) {
            lowestRadius = v23 * v23 + v27 * v27 + v12 * v12;
            closestId = i;
        }
    }

    if (lowestRadius != UINT_MAX) {
        *a2 = PID(OBJECT_Actor, closestId);
    }

    if (can_target_party && !pParty->Invisible()) {
        if (thisActor->ActorEnemy() &&
            !thisActor->pActorBuffs[ACTOR_BUFF_ENSLAVED].Active() &&
            !thisActor->pActorBuffs[ACTOR_BUFF_CHARM].Active() &&
            !thisActor->pActorBuffs[ACTOR_BUFF_SUMMONED].Active())
            v14 = 4;
        else
            v14 = thisActor->GetActorsRelation(0);
        if (v14 != 0) {
            if (!thisActor->pMonsterInfo.uHostilityType)
                v15 = _4DF380_hostilityRanges[v14];
            else
                v15 = _4DF380_hostilityRanges[4];
            v16 = abs(thisActor->vPosition.x - pParty->vPosition.x);
            v28 = abs(thisActor->vPosition.y - pParty->vPosition.y);
            v17 = abs(thisActor->vPosition.z - pParty->vPosition.z);
            if (v16 <= v15 && v28 <= v15 && v17 <= v15 &&
                (v16 * v16 + v28 * v28 + v17 * v17 < lowestRadius)) {
                *a2 = OBJECT_Player;
            }
        }
    }
}

//----- (0040104C) --------------------------------------------------------
signed int Actor::GetActorsRelation(Actor *otherActPtr) {
    unsigned int thisGroup;  // ebp@19
    int otherGroup;          // eax@22
    unsigned int thisAlly;   // edx@25
    unsigned int otherAlly;  // edx@33

    if (otherActPtr) {
        if (otherActPtr->uGroup != 0 && this->uGroup != 0 &&
            otherActPtr->uGroup == this->uGroup)
            return 0;
    }

    if (this->pActorBuffs[ACTOR_BUFF_BERSERK].Active()) return 4;
    thisAlly = this->uAlly;
    if (this->pActorBuffs[ACTOR_BUFF_ENSLAVED].Active() || thisAlly == 9999)
        thisGroup = 0;
    else if (thisAlly > 0)
        thisGroup = thisAlly;
    else
        thisGroup = (this->pMonsterInfo.uID - 1) / 3 + 1;

    if (otherActPtr) {
        if (otherActPtr->pActorBuffs[ACTOR_BUFF_BERSERK].Active()) return 4;
        otherAlly = otherActPtr->uAlly;
        if (otherActPtr->pActorBuffs[ACTOR_BUFF_ENSLAVED].Active() ||
            otherAlly == 9999)
            otherGroup = 0;
        else if (otherAlly > 0)
            otherGroup = otherAlly;
        else
            otherGroup = (otherActPtr->pMonsterInfo.uID - 1) / 3 + 1;
    } else {
        otherGroup = 0;
    }

    if (this->pActorBuffs[ACTOR_BUFF_CHARM].Active() && !otherGroup ||
        otherActPtr && otherActPtr->pActorBuffs[ACTOR_BUFF_CHARM].Active() &&
            !thisGroup)
        return 0;
    if (!this->pActorBuffs[ACTOR_BUFF_ENSLAVED].Active() &&
        this->ActorEnemy() && !otherGroup)
        return 4;
    if (thisGroup >= 89 || otherGroup >= 89) return 0;

    if (thisGroup == 0) {
        if ((!otherActPtr || this->pActorBuffs[ACTOR_BUFF_ENSLAVED].Active() &&
                                 otherActPtr->ActorFriend()) &&
            !pFactionTable->relations[otherGroup][0])
            return pFactionTable->relations[0][otherGroup];
        else
            return 4;
    } else {
        return pFactionTable->relations[thisGroup][otherGroup];
    }
}

//----- (0045976D) --------------------------------------------------------
void Actor::UpdateAnimation() {
    ResetAnimation();
    switch (uAIState) {
        case Tethered:
            uCurrentActionAnimation = ANIM_Walking;
            break;

        case AttackingMelee:
            uCurrentActionAnimation = ANIM_AtkMelee;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case AttackingRanged1:
        case AttackingRanged2:
        case AttackingRanged3:
        case AttackingRanged4:
            uCurrentActionAnimation = ANIM_AtkRanged;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Dying:
        case Resurrected:
            uCurrentActionAnimation = ANIM_Dying;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Pursuing:
        case Fleeing:
            uCurrentActionAnimation = ANIM_Walking;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Stunned:
            uCurrentActionAnimation = ANIM_GotHit;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Fidgeting:
            uCurrentActionAnimation = ANIM_Bored;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Standing:
        case Interacting:
        case Summoned:
            uCurrentActionAnimation = ANIM_Standing;
            uAttributes |= ACTOR_ANIMATION;
            break;

        case Dead:
            if (pSpriteFrameTable->pSpriteSFrames[pSpriteIDs[ANIM_Dead]]
                    .hw_sprites[0] == nullptr)
                uAIState = Removed;
            else
                uCurrentActionAnimation = ANIM_Dead;
            break;

        case Removed:
        case Disabled:
            return;

        default:
            assert(false);
    }
}

//----- (00459671) --------------------------------------------------------
void Actor::Reset() {
    this->pActorName[0] = 0;
    this->word_000086_some_monster_id = 0;
    this->sNPC_ID = 0;
    this->vPosition.z = 0;
    this->vPosition.y = 0;
    this->vPosition.x = 0;
    this->vVelocity.z = 0;
    this->vVelocity.y = 0;
    this->vVelocity.x = 0;
    this->uYawAngle = 0;
    this->uPitchAngle = 0;
    this->uAttributes = 0;
    this->uSectorID = 0;
    this->uCurrentActionTime = 0;
    this->vInitialPosition.z = 0;
    this->vInitialPosition.y = 0;
    this->vInitialPosition.x = 0;
    this->vGuardingPosition.z = 0;
    this->vGuardingPosition.y = 0;
    this->vGuardingPosition.x = 0;
    this->uTetherDistance = 256;
    this->uActorRadius = 32;
    this->uActorHeight = 128;
    this->uAIState = Standing;
    this->uCurrentActionAnimation = ANIM_Standing;
    this->uMovementSpeed = 200;
    this->uCarriedItemID = 0;
    this->uGroup = 0;
    this->uAlly = 0;
    this->uSummonerID = 0;
    this->uLastCharacterIDToHit = 0;
    this->dword_000334_unique_name = 0;
    memset(this->pSpriteIDs, 0, sizeof(pSpriteIDs));
    memset(this->pActorBuffs, 0, 0x160u);
}

//----- (0045959A) --------------------------------------------------------
void Actor::PrepareSprites(char load_sounds_if_bit1_set) {
    MonsterDesc *v3;  // esi@1
    MonsterInfo *v9;  // [sp+84h] [bp-10h]@1

    v3 = &pMonsterList->pMonsters[pMonsterInfo.uID - 1];
    v9 = &pMonsterStats->pInfos[pMonsterInfo.uID /*- 1 + 1*/];
    // v12 = pSpriteIDs;
    // Source = (char *)v3->pSpriteNames;
    // do
    for (uint i = 0; i < 8; ++i) {
        // strcpy(pSpriteName, v3->pSpriteNames[i]);
        pSpriteIDs[i] = pSpriteFrameTable->FastFindSprite(v3->pSpriteNames[i]);
        pSpriteFrameTable->InitializeSprite(pSpriteIDs[i]);
    }
    uActorHeight = v3->uMonsterHeight;
    uActorRadius = v3->uMonsterRadius;
    uMovementSpeed = v9->uBaseSpeed;
    if (!(load_sounds_if_bit1_set & 1)) {
        for (int i = 0; i < 4; ++i) pSoundSampleIDs[i] = v3->pSoundSampleIDs[i];
    }
}

//----- (00459667) --------------------------------------------------------
void Actor::Remove() { this->uAIState = Removed; }

//----- (0043B1B0) --------------------------------------------------------
void Actor::ActorDamageFromMonster(signed int attacker_id,
                                   unsigned int actor_id, Vec3_int_ *pVelocity,
                                   signed int a4) {
    int v4;            // ebx@1
    int dmgToRecv;     // qax@8
    signed int v12;    // ecx@20
    int finalDmg;      // edi@30
    int pushDistance;  // [sp+20h] [bp+Ch]@34

    v4 = 0;
    if (PID_TYPE(attacker_id) == OBJECT_Item) {
        v4 = pSpriteObjects[PID_ID(attacker_id)]
                 .field_60_distance_related_prolly_lod;
        attacker_id = pSpriteObjects[PID_ID(attacker_id)].spell_caster_pid;
    }
    if (PID_TYPE(attacker_id) == OBJECT_Actor) {
        if (!pActors[actor_id].IsNotAlive()) {
            pActors[actor_id].uLastCharacterIDToHit = attacker_id;
            if (pActors[actor_id].uAIState == Fleeing)
                pActors[actor_id].uAttributes |= ACTOR_FLEEING;
            if (pActors[PID_ID(attacker_id)]._4273BB_DoesHitOtherActor(
                    &pActors[actor_id], v4, 0)) {
                dmgToRecv = pActors[PID_ID(attacker_id)]._43B3E0_CalcDamage(a4);
                if (pActors[PID_ID(attacker_id)]
                        .pActorBuffs[ACTOR_BUFF_SHRINK]
                        .Active()) {
                    if (pActors[PID_ID(attacker_id)]
                            .pActorBuffs[ACTOR_BUFF_SHRINK]
                            .uPower > 0)
                        dmgToRecv =
                            dmgToRecv / pActors[PID_ID(attacker_id)]
                                            .pActorBuffs[ACTOR_BUFF_SHRINK]
                                            .uPower;
                }
                if (pActors[actor_id].pActorBuffs[ACTOR_BUFF_STONED].Active())
                    dmgToRecv = 0;
                if (a4 == 0) {
                    v12 =
                        pActors[PID_ID(attacker_id)].pMonsterInfo.uAttack1Type;
                } else if (a4 == 1) {
                    v12 =
                        pActors[PID_ID(attacker_id)].pMonsterInfo.uAttack2Type;
                    if (pActors[actor_id]
                            .pActorBuffs[ACTOR_BUFF_SHIELD]
                            .Active())
                        dmgToRecv = dmgToRecv / 2;
                } else if (a4 == 2) {
                    v12 = pSpellStats
                        ->pInfos[pActors[actor_id].pMonsterInfo.uSpell1ID]
                        .uSchool;
                } else if (a4 == 3) {
                    v12 = pSpellStats
                        ->pInfos[pActors[actor_id].pMonsterInfo.uSpell2ID]
                        .uSchool;
                } else if (a4 == 4) {
                    v12 = pActors[PID_ID(attacker_id)]
                        .pMonsterInfo.field_3C_some_special_attack;
                } else {
                    v12 = 4;
                }
                finalDmg = pActors[actor_id].CalcMagicalDamageToActor(
                    (DAMAGE_TYPE)v12, dmgToRecv);
                pActors[actor_id].sCurrentHP -= finalDmg;
                if (finalDmg) {
                    if (pActors[actor_id].sCurrentHP > 0)
                        Actor::AI_Stun(actor_id, attacker_id, 0);
                    else
                        Actor::Die(actor_id);
                    Actor::AggroSurroundingPeasants(actor_id, 0);
                    pushDistance =
                        20 * finalDmg / pActors[actor_id].pMonsterInfo.uHP;
                    if (pushDistance > 10) pushDistance = 10;
                    if (!MonsterStats::BelongsToSupertype(
                            pActors[actor_id].pMonsterInfo.uID,
                            MONSTER_SUPERTYPE_TREANT)) {
                        pVelocity->x =
                            (int32)fixpoint_mul(pushDistance, pVelocity->x);
                        pVelocity->y =
                            (int32)fixpoint_mul(pushDistance, pVelocity->y);
                        pVelocity->z =
                            (int32)fixpoint_mul(pushDistance, pVelocity->z);
                        pActors[actor_id].vVelocity.x =
                            50 * (short)pVelocity->x;
                        pActors[actor_id].vVelocity.y =
                            50 * (short)pVelocity->y;
                        pActors[actor_id].vVelocity.z =
                            50 * (short)pVelocity->z;
                    }
                    Actor::AddBloodsplatOnDamageOverlay(actor_id, 1, finalDmg);
                } else {
                    Actor::AI_Stun(actor_id, attacker_id, 0);
                }
                return;
            }
        }
    }
}

//----- (0044FD29) --------------------------------------------------------
void Actor::SummonMinion(int summonerId) {
    unsigned __int8 extraSummonLevel;  // al@1
    int summonMonsterBaseType;         // esi@1
    int v5;                            // edx@2
    int v7;                            // edi@10
    Actor *actor;                      // esi@10
    MonsterInfo *v9;                   // ebx@10
    // MonsterDesc *v10; // edi@10
    int v13;                 // ebx@10
    int64 v15;                 // edi@10
    int64 v17;                 // ebx@10
    unsigned int v19;        // qax@10
    int result;              // eax@13
    unsigned int monsterId;  // [sp+10h] [bp-18h]@8
    int v27;                 // [sp+18h] [bp-10h]@10
    int actorSector;         // [sp+1Ch] [bp-Ch]@8

    actorSector = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        actorSector = pIndoor->GetSector(this->vPosition.x, this->vPosition.y,
                                         this->vPosition.z);

    v19 = this->uAlly;
    if (!this->uAlly) {
        monsterId = this->pMonsterInfo.uID - 1;
        v19 = (uint)(monsterId * 0.33333334);
    }
    v27 = uCurrentlyLoadedLevelType == LEVEL_Outdoor ? 128 : 64;
    v13 = rand() % 2048;
    v15 = fixpoint_mul(stru_5C6E00->Cos(v13), v27) + this->vPosition.x;
    v17 = fixpoint_mul(stru_5C6E00->Sin(v13), v27) + this->vPosition.y;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        result = pIndoor->GetSector(v15, v17, this->vPosition.z);
        if (result != actorSector) return;
        result = BLV_GetFloorLevel(v15, v17, v27, result, &monsterId);
        if (result != -30000) return;
        if (abs(result - v27) > 1024) return;
    }

    extraSummonLevel = this->pMonsterInfo.uSpecialAbilityDamageDiceRolls;
    summonMonsterBaseType = this->pMonsterInfo.field_3C_some_special_attack;
    if (extraSummonLevel) {
        if (extraSummonLevel >= 1 && extraSummonLevel <= 3)
            summonMonsterBaseType =
                summonMonsterBaseType + extraSummonLevel - 1;
    } else {
        v5 = rand() % 100;
        if (v5 >= 90)
            summonMonsterBaseType += 2;
        else if (v5 >= 60)
            summonMonsterBaseType += 1;
    }
    v7 = summonMonsterBaseType - 1;
    actor = &pActors[uNumActors];
    v9 = &pMonsterStats->pInfos[v7 + 1];
    pActors[uNumActors].Reset();
    strcpy(actor->pActorName, v9->pName);
    actor->sCurrentHP = (short)v9->uHP;
    memcpy(&actor->pMonsterInfo, v9, sizeof(actor->pMonsterInfo));
    actor->word_000086_some_monster_id = summonMonsterBaseType;
    actor->uActorRadius = pMonsterList->pMonsters[v7].uMonsterRadius;
    actor->uActorHeight = pMonsterList->pMonsters[v7].uMonsterHeight;
    actor->pMonsterInfo.uTreasureDiceRolls = 0;
    actor->pMonsterInfo.uTreasureType = 0;
    actor->pMonsterInfo.uExp = 0;
    actor->uMovementSpeed = pMonsterList->pMonsters[v7].uMovementSpeed;

    actor->vInitialPosition.x = v15;
    actor->vInitialPosition.y = v17;
    actor->vInitialPosition.z = this->vPosition.z;
    actor->vPosition.x = v15;
    actor->vPosition.y = v17;
    actor->vPosition.z = this->vPosition.z;

    actor->uTetherDistance = 256;
    actor->uSectorID = actorSector;
    actor->PrepareSprites(0);
    actor->pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
    actor->uAlly = v19;
    actor->uCurrentActionTime = 0;
    actor->uGroup = this->uGroup;
    actor->uAIState = Summoned;
    actor->uCurrentActionLength = 256;
    actor->UpdateAnimation();

    ++uNumActors;
    ++this->pMonsterInfo.uSpecialAbilityDamageDiceBonus;
    if (ActorEnemy()) actor->uAttributes |= ACTOR_AGGRESSOR;
    actor->uSummonerID = PID(OBJECT_Actor, summonerId);
}
// 46DF1A: using guessed type int 46DF1A_collide_against_actor(int, int);
//----- (0046DF1A) --------------------------------------------------------
bool Actor::_46DF1A_collide_against_actor(int a1, int a2) {
    Actor *v2;            // edi@1
    unsigned __int16 v3;  // ax@1
    int v4;               // esi@6
    int v8;               // ecx@14
    int v9;               // eax@14
    int v10;              // ebx@14
    int v11;              // esi@14
    int v12;              // ebx@15
    int v13;              // ebx@17

    v2 = &pActors[a1];
    v3 = v2->uAIState;
    if (v3 == Removed || v3 == Dying || v3 == Disabled || v3 == Dead ||
        v3 == Summoned)
        return 0;
    v4 = v2->uActorRadius;
    if (a2) v4 = a2;

    if (stru_721530.sMaxX > v2->vPosition.x + v4 ||
        stru_721530.sMinX < v2->vPosition.x - v4 ||
        stru_721530.sMaxY > v2->vPosition.y + v4 ||
        stru_721530.sMinY < v2->vPosition.y - v4 ||
        stru_721530.sMaxZ > v2->vPosition.z + v2->uActorHeight ||
        stru_721530.sMinZ < v2->vPosition.z) {
        return false;
    }
    v8 = v2->vPosition.x - stru_721530.normal.x;
    v9 = v2->vPosition.y - stru_721530.normal.y;
    v10 = stru_721530.prolly_normal_d + v4;
    v11 = (v8 * stru_721530.direction.y - v9 * stru_721530.direction.x) >> 16;
    v12 = (v8 * stru_721530.direction.x + v9 * stru_721530.direction.y) >> 16;
    if (abs(v11) > v10 || v12 <= 0) return false;
    if (fixpoint_mul(stru_721530.direction.z, v12) + stru_721530.normal.z <
        v2->vPosition.z)
        return false;

    v13 = v12 - integer_sqrt(v10 * v10 - v11 * v11);
    if (v13 < 0) v13 = 0;
    if (v13 < stru_721530.field_7C) {
        stru_721530.field_7C = v13;
        stru_721530.pid = PID(OBJECT_Actor, a1);
    }
    return true;
}
//----- (00401A91) --------------------------------------------------------
void Actor::UpdateActorAI() {
    signed int v4;    // edi@10
    signed int sDmg;  // eax@14
    Player *pPlayer;  // ecx@21
    Actor *pActor;    // esi@34
    // unsigned __int16 v22; // ax@86
    unsigned int v27;        // ecx@123
    unsigned int v28;        // eax@123
    int v33;                 // eax@144
    int v34;                 // eax@147
    char v35;                // al@150
    unsigned int v36;        // edi@152
    signed int v37;          // eax@154
    double v42;              // st7@176
    double v43;              // st6@176
    int v45;                 // eax@192
    unsigned __int8 v46;     // cl@197
    signed int v47;          // st7@206
    uint v58;                // st7@246
    unsigned int v65;        // [sp-10h] [bp-C0h]@144
    int v70;                 // [sp-10h] [bp-C0h]@213
    AIDirection v72;         // [sp+0h] [bp-B0h]@246
    AIDirection a3;          // [sp+1Ch] [bp-94h]@129
    int target_pid_type;     // [sp+70h] [bp-40h]@83
    signed int a1;           // [sp+74h] [bp-3Ch]@129
    int v78;                 // [sp+78h] [bp-38h]@79
    AIDirection *pDir;       // [sp+7Ch] [bp-34h]@129
    float radiusMultiplier;  // [sp+98h] [bp-18h]@33
    int v81;                 // [sp+9Ch] [bp-14h]@100
    signed int target_pid;   // [sp+ACh] [bp-4h]@83
    AIState uAIState;
    uint v38;

    // Build AI array
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        Actor::MakeActorAIList_ODM();
    else
        Actor::MakeActorAIList_BLV();

    // Armageddon damage mechanic
    if (uCurrentlyLoadedLevelType != LEVEL_Indoor &&
        pParty->armageddon_timer > 0) {
        if (pParty->armageddon_timer > 417) {
            pParty->armageddon_timer = 0;
        } else {
            pParty->sRotationY = (stru_5C6E00->uIntegerDoublePi - 1) &
                                 (pParty->sRotationY + rand() % 16 - 8);
            pParty->sRotationX = pParty->sRotationX + rand() % 16 - 8;
            if (pParty->sRotationX > 128)
                pParty->sRotationX = 128;
            else if (pParty->sRotationX < -128)
                pParty->sRotationX = -128;

            pParty->uFlags |= 2u;
            pParty->armageddon_timer -= pMiscTimer->uTimeElapsed;
            v4 = pParty->armageddonDamage + 50;
            if (pParty->armageddon_timer <= 0) {
                pParty->armageddon_timer = 0;
                for (size_t i = 0; i < uNumActors; i++) {
                    pActor = &pActors[i];
                    if (pActor->CanAct()) {
                        sDmg = pActor->CalcMagicalDamageToActor((DAMAGE_TYPE)5,
                                                                v4);
                        pActor->sCurrentHP -= sDmg;
                        if (sDmg) {
                            if (pActor->sCurrentHP >= 0) {
                                Actor::AI_Stun(i, 4, 0);
                            } else {
                                Actor::Die(i);
                                if (pActor->pMonsterInfo.uExp)
                                    pParty->GivePartyExp(
                                        pMonsterStats
                                            ->pInfos[pActor->pMonsterInfo.uID]
                                            .uExp);
                            }
                        }
                    }
                }
                for (int i = 1; i <= 4; i++) {
                    pPlayer = pPlayers[i];
                    if (!pPlayer->conditions_times[Condition_Dead] &&
                        !pPlayer->conditions_times[Condition_Pertified] &&
                        !pPlayer->conditions_times[Condition_Eradicated])
                        pPlayer->ReceiveDamage(v4, DMGT_MAGICAL);
                }
            }
            if (pTurnEngine->pending_actions) --pTurnEngine->pending_actions;
        }
    }

    // Turn-based mode: return
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->AITurnBasedAction();
        return;
    }

    for (uint i = 0; i < uNumActors; ++i) {
        pActor = &pActors[i];
        ai_near_actors_targets_pid[i] = OBJECT_Player;

        // Skip actor if: Dead / Removed / Disabled / uAttributes & 0x0400
        if (pActor->uAIState == Dead || pActor->uAIState == Removed ||
            pActor->uAIState == Disabled || pActor->uAttributes & ACTOR_ALIVE)
            continue;

        // Kill actor if HP == 0
        if (!pActor->sCurrentHP && pActor->uAIState != Dying) Actor::Die(i);

        // Kill buffs if expired
        for (uint j = 0; j < 22; ++j) {
            if (j != 10)
                pActor->pActorBuffs[j].IsBuffExpiredToTime(
                    pParty->GetPlayingTime());
        }

        // If shrink expired: reset height
        if (pActor->pActorBuffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->uActorHeight =
                pMonsterList->pMonsters[pActor->pMonsterInfo.uID - 1]
                    .uMonsterHeight;
        }

        // If Charm still active: make actor friendly
        if (pActor->pActorBuffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
        } else if (pActor->pActorBuffs[ACTOR_BUFF_CHARM].Expired()) {
          // Else: reset hostilty
          pActor->pMonsterInfo.uHostilityType =
                pMonsterStats->pInfos[pActor->pMonsterInfo.uID].uHostilityType;
        }

        // If actor Paralyzed or Stoned: skip
        if (pActor->pActorBuffs[ACTOR_BUFF_PARALYZED].Active() ||
            pActor->pActorBuffs[ACTOR_BUFF_STONED].Active())
            continue;

        // Calculate RecoveryTime
        pActor->pMonsterInfo.uRecoveryTime = max(
            pActor->pMonsterInfo.uRecoveryTime - pMiscTimer->uTimeElapsed, 0);

        pActor->uCurrentActionTime += pMiscTimer->uTimeElapsed;
        if (pActor->uCurrentActionTime < pActor->uCurrentActionLength) continue;

        if (pActor->uAIState == Dying) {
            pActor->uAIState = Dead;
        } else {
            if (pActor->uAIState != Summoned) {
                Actor::AI_StandOrBored(i, OBJECT_Player, 256, nullptr);
                continue;
            }
            pActor->uAIState = Standing;
        }

        pActor->uCurrentActionTime = 0;
        pActor->uCurrentActionLength = 0;
        pActor->UpdateAnimation();
    }

    for (v78 = 0; v78 < ai_arrays_size; ++v78) {
        uint actor_id = ai_near_actors_ids[v78];
        assert(actor_id < uNumActors);

        pActor = &pActors[actor_id];

        v47 = (signed int)(pActor->pMonsterInfo.uRecoveryTime *
                           2.133333333333333);

        Actor::_SelectTarget(actor_id, &ai_near_actors_targets_pid[actor_id],
                             true);

        if (pActor->pMonsterInfo.uHostilityType &&
            !ai_near_actors_targets_pid[actor_id])
            pActor->pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;

        target_pid = ai_near_actors_targets_pid[actor_id];
        target_pid_type = PID_TYPE(target_pid);

        if (target_pid_type == OBJECT_Actor)
            radiusMultiplier = 0.5;
        else
            radiusMultiplier = 1.0;

        // v22 = pActor->uAIState;
        if (pActor->uAIState == Dying || pActor->uAIState == Dead ||
            pActor->uAIState == Removed || pActor->uAIState == Disabled ||
            pActor->uAIState == Summoned)
            continue;

        if (!pActor->sCurrentHP) Actor::Die(actor_id);

        for (int i = 0; i < 22; i++) {
            if (i != 10)
                pActor->pActorBuffs[i].IsBuffExpiredToTime(
                    pParty->GetPlayingTime());
        }

        if (pActor->pActorBuffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->uActorHeight =
                pMonsterList->pMonsters[pActor->pMonsterInfo.uID - 1]
                    .uMonsterHeight;
        }

        if (pActor->pActorBuffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
        } else if (pActor->pActorBuffs[ACTOR_BUFF_CHARM].Expired()) {
            pActor->pMonsterInfo.uHostilityType =
                pMonsterStats->pInfos[pActor->pMonsterInfo.uID].uHostilityType;
        }

        // If actor is summoned and buff expired: continue and set state to
        // Removed
        if (pActor->pActorBuffs[ACTOR_BUFF_SUMMONED].Expired()) {
            pActor->uAIState = Removed;
            continue;
        }

        if (pActor->pActorBuffs[ACTOR_BUFF_STONED].Active() ||
            pActor->pActorBuffs[ACTOR_BUFF_PARALYZED].Active()) {
            continue;
        }

        v27 = pMiscTimer->uTimeElapsed;
        v28 = pActor->pMonsterInfo.uRecoveryTime;
        pActor->uCurrentActionTime += pMiscTimer->uTimeElapsed;

        if ((signed int)v28 > 0) pActor->pMonsterInfo.uRecoveryTime = v28 - v27;
        if (pActor->pMonsterInfo.uRecoveryTime < 0)
            pActor->pMonsterInfo.uRecoveryTime = 0;
        if (!pActor->ActorNearby()) pActor->uAttributes |= ACTOR_NEARBY;

        a1 = PID(OBJECT_Actor, actor_id);
        Actor::GetDirectionInfo(PID(OBJECT_Actor, actor_id), target_pid, &a3,
                                0);
        pDir = &a3;
        uAIState = pActor->uAIState;

        if (pActor->pMonsterInfo.uHostilityType ==
                MonsterInfo::Hostility_Friendly ||
            (signed int)pActor->pMonsterInfo.uRecoveryTime > 0 ||
            radiusMultiplier * 307.2 < pDir->uDistance ||
            uAIState != Pursuing && uAIState != Standing &&
                uAIState != Tethered && uAIState != Fidgeting &&
                !pActor->pMonsterInfo.uMissleAttack1Type ||
            uAIState != Stunned) {
            if (pActor->uCurrentActionTime < pActor->uCurrentActionLength) {
                continue;
            } else if (pActor->uAIState == AttackingMelee) {
                v35 = pActor->special_ability_use_check(actor_id);
                AttackerInfo.Add(a1, 5120, pActor->vPosition.x,
                                 pActor->vPosition.y,
                                 pActor->vPosition.z +
                                     ((signed int)pActor->uActorHeight >> 1),
                                 v35, 1);
            } else if (pActor->uAIState == AttackingRanged1) {
                v34 = pActor->pMonsterInfo.uMissleAttack1Type;
                Actor::AI_RangedAttack(actor_id, pDir, v34, 0);  // light missile
            } else if (pActor->uAIState == AttackingRanged2) {
                v34 = pActor->pMonsterInfo.uMissleAttack2Type;
                Actor::AI_RangedAttack(actor_id, pDir, v34, 1);  // arrow
            } else if (pActor->uAIState == AttackingRanged3) {
                v65 = pActor->pMonsterInfo.uSpellSkillAndMastery1;
                v33 = pActor->pMonsterInfo.uSpell1ID;
                Actor::AI_SpellAttack(actor_id, pDir, v33, 2, v65);
            } else if (pActor->uAIState == AttackingRanged4) {
                v65 = pActor->pMonsterInfo.uSpellSkillAndMastery2;
                v33 = pActor->pMonsterInfo.uSpell2ID;
                Actor::AI_SpellAttack(actor_id, pDir, v33, 3, v65);
            }
        }

        v36 = pDir->uDistance;

        if (pActor->pMonsterInfo.uHostilityType ==
            MonsterInfo::Hostility_Friendly) {
            if (target_pid_type == OBJECT_Actor) {
                v36 = pDir->uDistance;
                v37 = pFactionTable->relations
                          [(pActor->pMonsterInfo.uID - 1) / 3 + 1]
                          [(pActors[PID_ID(target_pid)].pMonsterInfo.uID - 1) /
                               3 +
                           1];
            } else {
                v37 = 4;
            }
            v38 = 0;
            if (v37 == 2)
                v38 = 1024;
            else if (v37 == 3)
                v38 = 2560;
            else if (v37 == 4)
                v38 = 5120;
            if (v37 >= 1 && v37 <= 4 && v36 < v38 || v37 == 1)
                pActor->pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
        }

        // If actor afraid: flee or if out of range random move
        if (pActor->pActorBuffs[ACTOR_BUFF_AFRAID].Active()) {
            if ((signed int)v36 >= 10240)
                Actor::AI_RandomMove(actor_id, target_pid, 1024, 0);
            else
                Actor::AI_Flee(actor_id, target_pid, 0, pDir);
            continue;
        }

        if (pActor->pMonsterInfo.uHostilityType ==
                MonsterInfo::Hostility_Long &&
            target_pid) {
            if (pActor->pMonsterInfo.uAIType == 1) {
                if (pActor->pMonsterInfo.uMovementType ==
                    MONSTER_MOVEMENT_TYPE_STAIONARY) {
                    Actor::AI_Stand(actor_id, target_pid,
                        (uint)(pActor->pMonsterInfo.uRecoveryTime *
                            2.133333333333333),
                        pDir);
                } else {
                    Actor::AI_Flee(actor_id, target_pid, 0, pDir);
                    continue;
                }
            }
            if (!(pActor->uAttributes & ACTOR_FLEEING)) {
                if (pActor->pMonsterInfo.uAIType == 2 ||
                    pActor->pMonsterInfo.uAIType == 3) {
                    if (pActor->pMonsterInfo.uAIType == 2)
                        v43 =
                            (double)(signed int)pActor->pMonsterInfo.uHP * 0.2;
                    if (pActor->pMonsterInfo.uAIType == 3)
                        v43 =
                            (double)(signed int)pActor->pMonsterInfo.uHP * 0.1;
                    v42 = (double)pActor->sCurrentHP;
                    if (v43 > v42 && (signed int)v36 < 10240) {
                        Actor::AI_Flee(actor_id, target_pid, 0, pDir);
                        continue;
                    }
                }
            }

            v81 = v36 - pActor->uActorRadius;
            if (target_pid_type == OBJECT_Actor)
                v81 -= pActors[PID_ID(target_pid)].uActorRadius;
            if (v81 < 0) v81 = 0;
            // rand();
            pActor->uAttributes &= ~ACTOR_UNKNOW5;  // ~0x40000
            if (v81 < 5120) {
                v45 = pActor->special_ability_use_check(actor_id);
                if (v45 == 0) {
                    if (pActor->pMonsterInfo.uMissleAttack1Type) {
                        if ((int)pActor->pMonsterInfo.uRecoveryTime <= 0) {
                            Actor::AI_MissileAttack1(actor_id, target_pid,
                                pDir);
                        } else if (pActor->pMonsterInfo.uMovementType ==
                            MONSTER_MOVEMENT_TYPE_STAIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            if (radiusMultiplier * 307.2 > (double)v81)
                                Actor::AI_Stand(actor_id, target_pid, v47,
                                                pDir);
                            else
                                Actor::AI_Pursue1(actor_id, target_pid,
                                                  actor_id, v47, pDir);
                        }
                    } else {
                        if ((double)v81 >= radiusMultiplier * 307.2) {
                            if (pActor->pMonsterInfo.uMovementType ==
                                MONSTER_MOVEMENT_TYPE_STAIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47,
                                    pDir);
                            } else if (v81 >= 1024) {  // monsters
                                Actor::AI_Pursue3(actor_id, target_pid, 0,
                                    pDir);
                            } else {
                                v70 = (signed int)(radiusMultiplier * 307.2);
                                // monsters
                                // guard after player runs away
                                // follow player
                                Actor::AI_Pursue2(actor_id, target_pid, 0, pDir,
                                                  v70);
                            }
                        } else if ((signed int)
                                       pActor->pMonsterInfo.uRecoveryTime > 0) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            // monsters
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                } else if (v45 == 2 || v45 == 3) {
                    if (v45 == 2)
                        v46 = pActor->pMonsterInfo.uSpell1ID;
                    else
                        v46 = pActor->pMonsterInfo.uSpell2ID;
                    if (v46) {
                        if ((signed int)pActor->pMonsterInfo.uRecoveryTime <=
                            0) {
                            if (v45 == 2)
                                Actor::AI_SpellAttack1(actor_id, target_pid,
                                                       pDir);
                            else
                                Actor::AI_SpellAttack2(actor_id, target_pid,
                                                       pDir);
                        } else if (radiusMultiplier * 307.2 > (double)v81 ||
                            pActor->pMonsterInfo.uMovementType ==
                            MONSTER_MOVEMENT_TYPE_STAIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_Pursue1(actor_id, target_pid, actor_id,
                                v47, pDir);
                        }
                    } else {
                        if ((double)v81 >= radiusMultiplier * 307.2) {
                            if (pActor->pMonsterInfo.uMovementType ==
                                MONSTER_MOVEMENT_TYPE_STAIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47,
                                    pDir);
                            } else if (v81 >= 1024) {
                                Actor::AI_Pursue3(actor_id, target_pid, 256,
                                    pDir);
                            } else {
                                v70 = (signed int)(radiusMultiplier * 307.2);
                                Actor::AI_Pursue2(actor_id, target_pid, 0, pDir,
                                                  v70);
                            }
                        } else if ((signed int)
                                       pActor->pMonsterInfo.uRecoveryTime > 0) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                }
            }
        }

        if (pActor->pMonsterInfo.uHostilityType !=
                MonsterInfo::Hostility_Long ||
            !target_pid || v81 >= 5120 || v45 != 1) {
            if (pActor->pMonsterInfo.uMovementType ==
                MONSTER_MOVEMENT_TYPE_SHORT) {
                Actor::AI_RandomMove(actor_id, 4, 1024, 0);
            } else if (pActor->pMonsterInfo.uMovementType ==
                MONSTER_MOVEMENT_TYPE_MEDIUM) {
                Actor::AI_RandomMove(actor_id, 4, 2560, 0);
            } else if (pActor->pMonsterInfo.uMovementType ==
                MONSTER_MOVEMENT_TYPE_LONG) {
                Actor::AI_RandomMove(actor_id, 4, 5120, 0);
            } else if (pActor->pMonsterInfo.uMovementType ==
                MONSTER_MOVEMENT_TYPE_FREE) {
                Actor::AI_RandomMove(actor_id, 4, 10240, 0);
            } else if (pActor->pMonsterInfo.uMovementType ==
                     MONSTER_MOVEMENT_TYPE_STAIONARY) {
                Actor::GetDirectionInfo(a1, 4, &v72, 0);
                v58 = (uint)(pActor->pMonsterInfo.uRecoveryTime *
                             2.133333333333333);
                Actor::AI_Stand(actor_id, 4, v58, &v72);
            }
        } else if (!pActor->pMonsterInfo.uMissleAttack2Type) {
            if ((double)v81 >= radiusMultiplier * 307.2) {
                if (pActor->pMonsterInfo.uMovementType ==
                    MONSTER_MOVEMENT_TYPE_STAIONARY) {
                    Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                } else if (v81 >= 1024) {
                    Actor::AI_Pursue3(actor_id, target_pid, 256, pDir);
                } else {
                    v70 = (int)(radiusMultiplier * 307.2);
                    Actor::AI_Pursue2(actor_id, target_pid, 0, pDir, v70);
                }
            } else if ((signed int)pActor->pMonsterInfo.uRecoveryTime > 0) {
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            } else {
                Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
            }
        } else if ((signed int)pActor->pMonsterInfo.uRecoveryTime > 0) {
            if (radiusMultiplier * 307.2 > (double)v81 ||
                pActor->pMonsterInfo.uMovementType ==
                    MONSTER_MOVEMENT_TYPE_STAIONARY)
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            else
                Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
        } else {
            Actor::AI_MissileAttack2(actor_id, target_pid, pDir);
        }
    }
}

//----- (0044665D) --------------------------------------------------------
// uType:     0 -> any monster
//            1 -> uParam is GroupID
//            2 -> uParam is MonsterID
//            3 -> uParam is ActorID
// uNumAlive: 0 -> all must be alive
int IsActorAlive(unsigned int uType, unsigned int uParam,
                 unsigned int uNumAlive) {
    unsigned int uAliveActors;  // eax@6
    unsigned int uTotalActors;  // [sp+0h] [bp-4h]@1

    uTotalActors = 0;
    if (uType) {
        if (uType == 1) {
            uAliveActors = Actor::SearchActorByGroup(&uTotalActors, uParam);
        } else {
            if (uType == 2) {
                uAliveActors =
                    Actor::SearchActorByMonsterID(&uTotalActors, uParam);
            } else {
                if (uType != 3) return 0;
                uAliveActors = Actor::SearchActorByID(&uTotalActors, uParam);
            }
        }
    } else {
        uAliveActors = Actor::SearchAliveActors(&uTotalActors);
    }

    if (uNumAlive)
        return uAliveActors >= uNumAlive;
    else
        return uTotalActors == uAliveActors;
}
//----- (00408B54) --------------------------------------------------------
unsigned int Actor::SearchActorByID(unsigned int *pTotalActors,
                                    unsigned int a2) {
    // int v4; // eax@1
    unsigned int result;  // ebx@1

    // v4 = GetAlertStatus();
    *pTotalActors = 0;
    result = 0;
    if ((pActors[a2].uAttributes & ACTOR_UNKNOW7) == GetAlertStatus()) {
        *pTotalActors = 1;
        if (pActors[a2].IsNotAlive() == 1) result = 1;
    }
    return result;
}
//----- (00408AE7) --------------------------------------------------------
unsigned int Actor::SearchActorByGroup(unsigned int *pTotalActors,
                                       unsigned int uGroup) {
    unsigned int result;  // [sp+10h] [bp-4h]@1

    int v8 = GetAlertStatus();
    *pTotalActors = 0;
    result = 0;
    for (uint i = 0; i < uNumActors; i++) {
        if ((pActors[i].uAttributes & ACTOR_UNKNOW7) == v8 &&
            pActors[i].uGroup == uGroup) {
            ++*pTotalActors;
            if (pActors[i].IsNotAlive() == 1) ++result;
        }
    }
    return result;
}
//----- (00408A7E) --------------------------------------------------------
unsigned int Actor::SearchActorByMonsterID(unsigned int *pTotalActors,
                                           int uMonsterID) {
    int v8;               // [sp+Ch] [bp-8h]@1
    unsigned int result;  // [sp+10h] [bp-4h]@1

    v8 = GetAlertStatus();
    *pTotalActors = 0;
    result = 0;
    for (uint i = 0; i < uNumActors; i++) {
        if ((pActors[i].uAttributes & ACTOR_UNKNOW7) == v8 &&
            pActors[i].pMonsterInfo.field_33 == uMonsterID) {
            ++*pTotalActors;
            if (pActors[i].IsNotAlive() == 1) ++result;
        }
    }
    return result;
}
//----- (00408A27) --------------------------------------------------------
unsigned int Actor::SearchAliveActors(unsigned int *pTotalActors) {
    int v2;               // eax@1
    unsigned int result;  // ebp@1

    v2 = GetAlertStatus();
    result = 0;
    *pTotalActors = 0;
    for (uint i = 0; i < uNumActors; i++) {
        if ((pActors[i].uAttributes & ACTOR_UNKNOW7) == v2) {
            ++*pTotalActors;
            if (pActors[i].IsNotAlive() == 1) ++result;
        }
    }
    return result;
}
//----- (00408768) --------------------------------------------------------
void Actor::InitializeActors() {
    bool bCelestia = false;
    bool bPit = false;
    bool good = false;
    bool evil = false;
    if (pCurrentMapName == "d25.blv") {  // the Celestia
        bCelestia = true;
    }
    if (pCurrentMapName == "d26.blv") {  // the Pit
        bPit = true;
    }
    if (pParty->IsPartyGood()) good = true;
    if (pParty->IsPartyEvil()) evil = true;

    logger->Warning(
        L"%S %S %u", __FILE__, __FUNCTION__,
        __LINE__);  // ai_near_actors_targets_pid[i] for AI_Stand seems always
                    // 0;  original code behaviour is identical
    for (uint i = 0; i < uNumActors; ++i) {
        Actor *actor = &pActors[i];

        if (actor->CanAct() || actor->uAIState == Disabled) {
            actor->vPosition.x = actor->vInitialPosition.x;
            actor->vPosition.y = actor->vInitialPosition.y;
            actor->vPosition.z = actor->vInitialPosition.z;
            actor->sCurrentHP = actor->pMonsterInfo.uHP;
            if (actor->uAIState != Disabled) {
                Actor::AI_Stand(i, ai_near_actors_targets_pid[i],
                                actor->pMonsterInfo.uRecoveryTime, 0);
            }
        }

        actor->pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;

        if (!bCelestia || good)
            if (!bPit || evil)
                if (actor->IsPeasant()) actor->ResetAggressor();  // ~0x80000

        actor->ResetHasItem();  // ~0x800000
        if (actor->uAttributes & ACTOR_UNKNOW9)
            Actor::_4031C1_update_job_never_gets_called(i, pParty->uCurrentHour,
                                                        1);
    }
}
//----- (00439474) --------------------------------------------------------
void Actor::DamageMonsterFromParty(signed int a1, unsigned int uActorID_Monster,
                                   Vec3_int_ *pVelocity) {
    SpriteObject *projectileSprite;  // ebx@1
    Actor *pMonster;                 // esi@7
    unsigned __int16 v16;            // cx@25
    int v33;                         // eax@100
    int v40;                         // ebx@107
    int extraRecoveryTime;           // qax@125
    unsigned __int16 v43;            // ax@132
    unsigned __int16 v45;            // ax@132
    // unsigned __int64 v46; // [sp+Ch] [bp-60h]@6
    char *pPlayerName;                // [sp+18h] [bp-54h]@12
    char *pMonsterName;               // [sp+1Ch] [bp-50h]@6
    signed int a4;                    // [sp+44h] [bp-28h]@1
    bool IsAdditionalDamagePossible;  // [sp+50h] [bp-1Ch]@1
    int v61;                          // [sp+58h] [bp-14h]@1
    bool isLifeStealing;              // [sp+5Ch] [bp-10h]@1
    int uDamageAmount;                // [sp+60h] [bp-Ch]@1
    DAMAGE_TYPE attackElement;        // [sp+64h] [bp-8h]@27

    projectileSprite = 0;
    uDamageAmount = 0;
    a4 = 0;
    v61 = 0;
    IsAdditionalDamagePossible = false;
    isLifeStealing = 0;
    if (PID_TYPE(a1) == OBJECT_Item) {
        projectileSprite = &pSpriteObjects[PID_ID(a1)];
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        a1 = projectileSprite->spell_caster_pid;
    }
    if (PID_TYPE(a1) != OBJECT_Player) return;

    assert(PID_ID(abs(a1)) < 4);
    Player *player = &pParty->pPlayers[PID_ID(a1)];
    pMonster = &pActors[uActorID_Monster];
    if (pMonster->IsNotAlive()) return;

    pMonster->uAttributes |= 0xC000;
    if (pMonster->uAIState == Fleeing) pMonster->uAttributes |= ACTOR_FLEEING;
    bool hit_will_stun = false, hit_will_paralyze = false;
    if (!projectileSprite) {
        int main_hand_idx = player->pEquipment.uMainHand;
        IsAdditionalDamagePossible = true;
        if (player->HasItemEquipped(EQUIP_TWO_HANDED)) {
            uint main_hand_skill =
                player->GetMainHandItem()->GetPlayerSkillType();
            uint main_hand_mastery =
                SkillToMastery(player->pActiveSkills[main_hand_skill]);
            switch (main_hand_skill) {
                case PLAYER_SKILL_STAFF:
                    if (main_hand_mastery >= 3) {
                        if (rand() % 100 <
                            (player->GetActualSkillLevel(PLAYER_SKILL_STAFF) &
                             0x3F))  // stun chance when mastery >= 3
                            hit_will_stun = true;
                    }
                    break;

                case PLAYER_SKILL_MACE:
                    if (main_hand_mastery >= 3) {
                        if (rand() % 100 <
                            (player->GetActualSkillLevel(PLAYER_SKILL_MACE) &
                             0x3F))
                            hit_will_stun = true;
                    }
                    if (main_hand_mastery >= 4) {
                        if (rand() % 100 <
                            (player->GetActualSkillLevel(PLAYER_SKILL_MACE) &
                             0x3F))
                            hit_will_paralyze = true;
                    }
                    break;
            }
        }
        attackElement = DMGT_PHISYCAL;
        uDamageAmount = player->CalculateMeleeDamageTo(
            false, false, pMonster->pMonsterInfo.uID);
        if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
            player->PlaySound(SPEECH_52, 0);
            return;
        }
    } else {
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        if (projectileSprite->spell_id != SPELL_DARK_SOULDRINKER) {
            int d1 = abs(pParty->vPosition.x - projectileSprite->vPosition.x);
            int d2 = abs(pParty->vPosition.y - projectileSprite->vPosition.y);
            int d3 = abs(pParty->vPosition.z - projectileSprite->vPosition.z);
            v61 = int_get_vector_length(d1, d2, d3);

            if (v61 >= 5120 && !(pMonster->uAttributes & ACTOR_ALIVE))  // 0x400
                return;
            else if (v61 >= 2560)
                v61 = 2;
            else
                v61 = 1;
        }

        switch (projectileSprite->spell_id) {
            case SPELL_LASER_PROJECTILE:
                v16 = player->pActiveSkills[PLAYER_SKILL_BLASTER];
                v61 = 1;
                if (SkillToMastery(v16) >= 3)
                    a4 = player->pActiveSkills[PLAYER_SKILL_BLASTER] & 0x3F;
                attackElement = DMGT_PHISYCAL;
                uDamageAmount = player->CalculateMeleeDamageTo(true, true, 0);
                if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
                    player->PlaySound(SPEECH_52, 0);
                    return;
                }
                break;
            case SPELL_101:
                attackElement = DMGT_FIRE;
                uDamageAmount = player->CalculateRangedDamageTo(0);
                if (pMonster->pActorBuffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = true;
                if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
                    player->PlaySound(SPEECH_52, 0);
                    return;
                }
                break;
            case SPELL_EARTH_BLADES:
                a4 = 5 * projectileSprite->spell_level;
                attackElement =
                    (DAMAGE_TYPE)player->GetSpellSchool(SPELL_EARTH_BLADES);
                uDamageAmount = _43AFE3_calc_spell_damage(
                    39, projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->sCurrentHP);
                if (pMonster->pActorBuffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = false;
                if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
                    player->PlaySound(SPEECH_52, 0);
                    return;
                }
                break;
            case SPELL_EARTH_STUN:
                uDamageAmount = 0;
                attackElement = DMGT_PHISYCAL;
                hit_will_stun = 1;
                if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
                    player->PlaySound(SPEECH_52, 0);
                    return;
                }
                break;
            case SPELL_BOW_ARROW:
                attackElement = DMGT_PHISYCAL;
                uDamageAmount = player->CalculateRangedDamageTo(
                    pMonster->word_000086_some_monster_id);
                if (pMonster->pActorBuffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount /= 2;
                IsAdditionalDamagePossible = true;
                if (projectileSprite->containing_item.uItemID != 0 &&
                    projectileSprite->containing_item.special_enchantment ==
                        3) {  // of carnage
                    attackElement = DMGT_FIRE;
                } else if (!player->PlayerHitOrMiss(pMonster, v61, a4)) {
                    player->PlaySound(SPEECH_52, 0);
                    return;
                }
                break;

            default:
                attackElement = (DAMAGE_TYPE)player->GetSpellSchool(
                    projectileSprite->spell_id);
                IsAdditionalDamagePossible = false;
                uDamageAmount = _43AFE3_calc_spell_damage(
                    projectileSprite->spell_id, projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->sCurrentHP);
                break;
        }
    }

    if (player->IsWeak()) uDamageAmount /= 2;
    if (pMonster->pActorBuffs[ACTOR_BUFF_STONED].Active()) uDamageAmount = 0;
    v61 = pMonster->CalcMagicalDamageToActor(attackElement, uDamageAmount);
    if (!projectileSprite && player->IsUnarmed() &&
        player->pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS].Active()) {
        v61 += pMonster->CalcMagicalDamageToActor(
            (DAMAGE_TYPE)8,
            player->pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS].uPower);
    }
    uDamageAmount = v61;
    if (IsAdditionalDamagePossible) {
        if (projectileSprite) {
            a4 =
                projectileSprite->containing_item._439DF3_get_additional_damage(
                    &attackElement, &isLifeStealing);
            if (isLifeStealing && pMonster->sCurrentHP > 0) {
                player->sHealth += v61 / 5;
                if (player->sHealth > player->GetMaxHealth())
                    player->sHealth = player->GetMaxHealth();
            }
            uDamageAmount +=
                pMonster->CalcMagicalDamageToActor(attackElement, a4);
        } else {
            for (int i = 0; i < 2; i++) {
                if (player->HasItemEquipped((ITEM_EQUIP_TYPE)i)) {
                    ItemGen *item;
                    if (i == 0)
                        item = player->GetOffHandItem();
                    else
                        item = player->GetMainHandItem();
                    a4 = item->_439DF3_get_additional_damage(&attackElement,
                                                             &isLifeStealing);
                    if (isLifeStealing && pMonster->sCurrentHP > 0) {
                        player->sHealth += v61 / 5;
                        if (player->sHealth > player->GetMaxHealth())
                            player->sHealth = player->GetMaxHealth();
                    }
                    uDamageAmount +=
                        pMonster->CalcMagicalDamageToActor(attackElement, a4);
                }
            }
        }
    }
    pMonster->sCurrentHP -= uDamageAmount;
    if (uDamageAmount == 0 && !hit_will_stun) {
        player->PlaySound(SPEECH_52, 0);
        return;
    }
    if (pMonster->sCurrentHP > 0) {
        Actor::AI_Stun(uActorID_Monster, a1, 0);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (!engine->config->NoShowDamage()) {
            String str;
            if (projectileSprite)
                str = localization->FormatString(
                    189, player->pName, pMonster->pActorName,
                    uDamageAmount);  // "%s shoots %s for %lu points"
            else
                str = localization->FormatString(
                    164, player->pName, pMonster->pActorName,
                    uDamageAmount);  // "%s hits %s for %lu damage"
            GameUI_StatusBar_OnEvent(str);
        }
    } else {
        if (pMonsterStats->pInfos[pMonster->pMonsterInfo.uID].bQuestMonster & 1) {
            if (!engine->config->NoBloodsplats()) {
                v33 = _4D864C_force_sw_render_rules && !engine->config->NoHugeBloodsplats()
                          ? 10 * pMonster->uActorRadius
                          : pMonster->uActorRadius;
                decal_builder->AddBloodsplat((float)pMonster->vPosition.x,
                                             (float)pMonster->vPosition.y,
                                             (float)pMonster->vPosition.z, 1.0,
                                             0.0, 0.0, (float)v33, 0, 0);
            }
        }
        Actor::Die(uActorID_Monster);
        Actor::ApplyFineForKillingPeasant(uActorID_Monster);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (pMonster->pMonsterInfo.uExp)
            pParty->GivePartyExp(
                pMonsterStats->pInfos[pMonster->pMonsterInfo.uID].uExp);
        v40 = SPEECH_51;
        if (rand() % 100 < 20)
            v40 = ((signed int)pMonster->pMonsterInfo.uHP >= 100) + 1;
        player->PlaySound((PlayerSpeech)v40, 0);
        if (!engine->config->NoShowDamage()) {
            pMonsterName = (char *)uDamageAmount;
            pPlayerName = player->pName;

            auto str = localization->FormatString(
                175, player->pName, uDamageAmount,
                pMonster);  // "%s inflicts %lu points killing %s"
            GameUI_StatusBar_OnEvent(str);
        }
    }
    if (pMonster->pActorBuffs[ACTOR_BUFF_PAIN_REFLECTION].Active() &&
        uDamageAmount != 0)
        player->ReceiveDamage(uDamageAmount, attackElement);
    int knockbackValue = 20 * v61 / (signed int)pMonster->pMonsterInfo.uHP;
    if ((player->GetSpecialItemBonus(ITEM_ENCHANTMENT_OF_FORCE) ||
         hit_will_stun) &&
        pMonster->DoesDmgTypeDoDamage(DMGT_EARTH)) {
        extraRecoveryTime = 20;
        knockbackValue = 10;
        if (!pParty->bTurnBasedModeOn)
            extraRecoveryTime =
                (int)(flt_6BE3A8_debug_recmod2 * 42.66666666666666);
        pMonster->pMonsterInfo.uRecoveryTime += extraRecoveryTime;
        if (!engine->config->NoShowDamage()) {
            pMonsterName = player->pName;

            auto str = localization->FormatString(635, player->pName,
                                                  pMonster);  // "%s stuns %s"
            GameUI_StatusBar_OnEvent(str);
        }
    }
    if (hit_will_paralyze && pMonster->CanAct() &&
        pMonster->DoesDmgTypeDoDamage(DMGT_EARTH)) {
        v43 = player->GetActualSkillLevel(PLAYER_SKILL_MACE);
        v45 = SkillToMastery(v43);
        GameTime v46 = GameTime(pParty->GetPlayingTime() + GameTime(0, v43 & 63));  // ??
        pMonster->pActorBuffs[ACTOR_BUFF_PARALYZED].Apply(v46, v45, 0, 0, 0);
        if (!engine->config->NoShowDamage()) {
            pMonsterName = player->pName;

            auto str = localization->FormatString(
                636, player->pName, pMonster);  // "%s paralyzes %s"
            GameUI_StatusBar_OnEvent(str);
        }
    }
    if (knockbackValue > 10) knockbackValue = 10;
    if (!MonsterStats::BelongsToSupertype(pMonster->pMonsterInfo.uID,
                                          MONSTER_SUPERTYPE_TREANT)) {
        pVelocity->x = fixpoint_mul(knockbackValue, pVelocity->x);
        pVelocity->y = fixpoint_mul(knockbackValue, pVelocity->y);
        pVelocity->z = fixpoint_mul(knockbackValue, pVelocity->z);
        pMonster->vVelocity.x = 50 * (short)pVelocity->x;
        pMonster->vVelocity.y = 50 * (short)pVelocity->y;
        pMonster->vVelocity.z = 50 * (short)pVelocity->z;
    }
    Actor::AddBloodsplatOnDamageOverlay(uActorID_Monster, 1, v61);
}

//----- (004BBF61) --------------------------------------------------------
void Actor::Arena_summon_actor(int monster_id, __int16 x, int y, int z) {
    // int v12;      // ebx@7
    // int v13;      // eax@8
    __int16 v16;  // [sp+10h] [bp-4h]@3

    if (uNumActors < 500) {
        v16 = 0;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
            v16 = pIndoor->GetSector(x, y, z);
        pActors[uNumActors].Reset();
        strcpy(pActors[uNumActors].pActorName,
               pMonsterStats->pInfos[monster_id].pName);
        pActors[uNumActors].sCurrentHP =
            (short)pMonsterStats->pInfos[monster_id].uHP;
        memcpy(&pActors[uNumActors].pMonsterInfo,
               &pMonsterStats->pInfos[monster_id], 0x58u);
        pActors[uNumActors].word_000086_some_monster_id = monster_id;
        pActors[uNumActors].uActorRadius =
            pMonsterList->pMonsters[monster_id - 1].uMonsterRadius;
        pActors[uNumActors].uActorHeight =
            pMonsterList->pMonsters[monster_id - 1].uMonsterHeight;
        pActors[uNumActors].uMovementSpeed =
            pMonsterList->pMonsters[monster_id - 1].uMovementSpeed;
        pActors[uNumActors].vInitialPosition.x = x;
        pActors[uNumActors].vPosition.x = x;
        pActors[uNumActors].uAttributes |= ACTOR_AGGRESSOR;
        pActors[uNumActors].pMonsterInfo.uTreasureType = 0;
        pActors[uNumActors].pMonsterInfo.uTreasureLevel = 0;
        pActors[uNumActors].pMonsterInfo.uTreasureDiceSides = 0;
        pActors[uNumActors].pMonsterInfo.uTreasureDiceRolls = 0;
        pActors[uNumActors].pMonsterInfo.uTreasureDropChance = 0;
        pActors[uNumActors].vInitialPosition.y = y;
        pActors[uNumActors].vPosition.y = y;
        pActors[uNumActors].vInitialPosition.z = z;
        pActors[uNumActors].vPosition.z = z;
        pActors[uNumActors].uTetherDistance = 256;
        pActors[uNumActors].uSectorID = v16;
        pActors[uNumActors].uGroup = 1;
        pActors[uNumActors].pMonsterInfo.uHostilityType =
            MonsterInfo::Hostility_Long;
        pActors[uNumActors].PrepareSprites(0);
        //    for ( int i = 0; i < 4; i++)
        //      pSoundList->LoadSound(pMonsterList->pMonsters[monster_id -
        //      1].pSoundSampleIDs[i], 0);
        //    v12 = 0;
        //    do
        //    {
        //      v13 = pSoundList->LoadSound(v12 +
        //      word_4EE088_sound_ids[pMonsterStats->pInfos[monster_id].uSpell1ID],
        //      1); v12++;
        //    }
        //    while ( v13 );
        ++uNumActors;
    }
}

//----- (00426E10) --------------------------------------------------------
int stru319::which_player_to_attack(Actor *pActor) {
    signed int v2;         // ebx@1
    bool flag;             // edi@37
    int v22;               // [sp+8h] [bp-140h]@3
    int Victims_list[60];  // [sp+48h] [bp-100h]@48
    int for_sex;           // [sp+13Ch] [bp-Ch]@1
    int for_race;          // [sp+140h] [bp-8h]@1
    int for_class;         // [sp+144h] [bp-4h]@1

    for_class = -1;
    for_race = -1;
    for_sex = -1;
    v2 = 0;
    if (pActor->pMonsterInfo.uAttackPreference) {
        for (uint i = 0; i < 16; i++) {
            v22 = pActor->pMonsterInfo.uAttackPreference & (1 << i);
            if (v22) {
                switch (v22) {
                    case 1:
                        for_class = 0;
                        break;
                    case 2:
                        for_class = 12;
                        break;
                    case 4:
                        for_class = 16;
                        break;
                    case 8:
                        for_class = 28;
                        break;
                    case 16:
                        for_class = 24;
                        break;
                    case 32:
                        for_class = 32;
                        break;
                    case 64:
                        for_class = 20;
                        break;
                    case 128:
                        for_class = 4;
                        break;
                    case 256:
                        for_class = 8;
                        break;
                    case 512:
                        for_sex = 0;
                        break;
                    case 1024:
                        for_sex = 1;
                        break;
                    case 2048:
                        for_race = 0;
                        break;
                    case 4096:
                        for_race = 1;
                        break;
                    case 8192:
                        for_race = 3;
                        break;
                    case 16384:
                        for_race = 2;
                        break;
                }
                v2 = 0;
                for (uint j = 0; j < 4; ++j) {
                    flag = 0;
                    if (for_class != -1 &&
                        for_class == pPlayers[j + 1]->classType)
                        flag = true;
                    if (for_sex != -1 && for_sex == pPlayers[j + 1]->uSex)
                        flag = true;
                    if (for_race != -1 &&
                        for_race == pPlayers[j + 1]->GetRace())
                        flag = true;
                    if (flag == true) {
                        if (!(pPlayers[j + 1]
                                  ->conditions_times[Condition_Paralyzed] ||
                              pPlayers[j + 1]
                                  ->conditions_times[Condition_Unconcious] ||
                              pPlayers[j + 1]
                                  ->conditions_times[Condition_Dead] ||
                              pPlayers[j + 1]
                                  ->conditions_times[Condition_Pertified] ||
                              pPlayers[j + 1]
                                  ->conditions_times[Condition_Eradicated]))
                            Victims_list[v2++] = j;
                    }
                }
            }
        }
        if (v2) return Victims_list[rand() % v2];
    }
    for (uint i = 0; i < 4; ++i) {
        if (!(pPlayers[i + 1]->conditions_times[Condition_Paralyzed] ||
              pPlayers[i + 1]->conditions_times[Condition_Unconcious] ||
              pPlayers[i + 1]->conditions_times[Condition_Dead] ||
              pPlayers[i + 1]->conditions_times[Condition_Pertified] ||
              pPlayers[i + 1]->conditions_times[Condition_Eradicated]))
            Victims_list[v2++] = i;
    }
    if (v2)
        return Victims_list[rand() % v2];
    else
        return 0;
}

//----- (00427546) --------------------------------------------------------
int stru319::_427546(int a2) {
    int result;  // eax@2

    if (a2 >= 0) {
        if (a2 >= 1)
            result = (a2 >= 2) + 2;
        else
            result = 1;
    } else {
        result = 0;
    }
    return result;
}
//----- (0042F184) --------------------------------------------------------
int stru319::FindClosestActor(int pick_depth, int a3, int a4) {
    int v4;       // edi@1
    stru319 *v5;  // esi@1
    int v6;       // eax@2
    int v7;       // eax@4
    //  int result; // eax@5
    //  int *v9; // edx@8
    //  signed int v10; // ebx@10
    //  int v11; // edi@11
    // Actor *v12; // esi@12
    // unsigned __int16 v13; // ax@12
    //  int v14; // eax@22
    // char v15; // zf@30
    //  int v16; // esi@32
    //  int v17; // ecx@34
    //  stru319 *v18; // eax@39
    //  int v19; // edx@39
    //  int v20; // ecx@41
    //  unsigned __int16 v21; // ax@42
    //  unsigned int v22; // [sp+8h] [bp-24h]@11
    // unsigned int v23; // [sp+Ch] [bp-20h]@7
    stru319 *v24;  // [sp+10h] [bp-1Ch]@1
    //  unsigned int v25; // [sp+14h] [bp-18h]@8
    //  int *v26; // [sp+18h] [bp-14h]@8
    //  int v27; // [sp+1Ch] [bp-10h]@10
    //  int *v28; // [sp+20h] [bp-Ch]@10
    // unsigned int v29; // [sp+24h] [bp-8h]@7
    //  int v30; // [sp+28h] [bp-4h]@6
    //  int i; // [sp+38h] [bp+Ch]@33
    //  signed int v32; // [sp+3Ch] [bp+10h]@32

    v4 = 0;
    v5 = this;
    v24 = this;
    // if ( render->pRenderD3D )
    {
        v6 = a3 != 0;
        if (a4) v6 |= 8;
        v7 = vis->PickClosestActor(OBJECT_Actor, pick_depth, v6, 657456, -1);
        if (v7 != -1)
            return (unsigned __int16)v7;
        else
            return 0;
    }
    /*else // software impl
    {
    v30 = 0;
    if ( render->pActiveZBuffer )
    {
    if ( (signed int)viewparams->uScreen_topL_Y < (signed
    int)viewparams->uScreen_BttmR_Y )
    {
    v9 = &render->pActiveZBuffer[viewparams->uScreen_topL_X + 640 *
    viewparams->uScreen_topL_Y]; v26 =
    &render->pActiveZBuffer[viewparams->uScreen_topL_X + 640 *
    viewparams->uScreen_topL_Y]; for ( v25 = viewparams->uScreen_BttmR_Y -
    viewparams->uScreen_topL_Y; v25; --v25 )
    {
    if ( (signed int)viewparams->uScreen_topL_X < (signed
    int)viewparams->uScreen_BttmR_X )
    {
    v28 = v9;
    v10 = v4;
    for ( v27 = viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X; v27;
    --v27 )
    {
    v22 = *v28;
    v11 = *v28 & 0xFFFF;
    if (PID_TYPE(v11) == OBJECT_Actor)
    {
    if ( pActors[PID_ID(v11)].uAIState != Dead )
    {
    if ( pActors[PID_ID(v11)].uAIState != Dying && pActors[PID_ID(v11)].uAIState
    != Removed
    && pActors[PID_ID(v11)].uAIState != Summoned &&
    pActors[PID_ID(v11)].uAIState != Disabled
    && (!a3 || pActors[PID_ID(v11)].GetActorsRelation(0)) )
    {
    if ( (!a4 ||
    MonsterStats::BelongsToSupertype(pActors[PID_ID(v11)].pMonsterInfo.uID,
    MONSTER_SUPERTYPE_UNDEAD))
    && v22 <= pick_depth << 16 )
    {
    v14 = 0;
    if ( v10 > 0 )
    {
    for ( v14; v14 < v30; ++v14 )
    {
    if ( dword_50BDA0[v14] == v11 )
    break;
    }
    }
    if ( v14 == v30 && v10 < 100 )
    {
    ++v30;
    dword_50BC10[v10] = v22;
    dword_50BDA0[v10] = v11;
    ++v10;
    }
    }
    }
    }
    }
    ++v28;
    }
    v4 = v30;
    v5 = v24;
    }
    v9 = v26 + 640;
    v26 += 640;
    }
    }
    if ( v4 > 0 )
    {
    v16 = (int)dword_50BC10.data();
    for ( v32 = 1; v32 - 1 < v4; ++v32 )
    {
    for ( i = v32; i < v4; ++i )
    {
    v17 = dword_50BC10[i];
    if ( dword_50BC10[i] < *(int *)v16 )
    {
    dword_50BC10[i] = *(int *)v16;
    *(int *)v16 = v17;
    }
    }
    v16 += 4;
    }
    v5 = v24;
    if ( v4 > 0 )
    {
    v18 = v24;
    for ( v19 = v4; v19; --v19 )
    {
    *(int *)&v18->field_0 = (*(int *)&v18[(char *)dword_50BC10.data() - (char
    *)v24].field_0 >> 3) & 0x1FFF; v18 += 4;
    }
    }
    }
    v20 = 0;
    for ( *(int *)&v5[2000].field_0 = v4; v20 < v4; ++v20 )
    {
    v21 = pActors[*(int *)&v5[4 * v20].field_0].uAIState;
    if ( v21 != 4 && v21 != 5 )
    break;
    }
    if ( v20 != v4 )
    {
    result = 8 * *(int *)&v5[4 * v20].field_0;
    LOBYTE(result) = result | 3;
    return result;
    }
    }
    }
    return 0;*/
}

//----- (0042F4DA) --------------------------------------------------------
bool CheckActors_proximity() {
    unsigned int distance;  // edi@1
    int for_x;            // ebx@5
    int for_y;            // [sp+Ch] [bp-10h]@5
    int for_z;            // [sp+10h] [bp-Ch]@5

    distance = 5120;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) distance = 2560;

    if ((signed int)uNumActors <= 0) return false;
    for (uint i = 0; i < (signed int)uNumActors; ++i) {
        for_x = abs(pActors[i].vInitialPosition.x - pParty->vPosition.x);
        for_y = abs(pActors[i].vInitialPosition.y - pParty->vPosition.y);
        for_z = abs(pActors[i].vInitialPosition.z - pParty->vPosition.z);
        if (int_get_vector_length(for_x, for_y, for_z) < distance) {
            if (pActors[i].uAIState != Dead) {
                if (pActors[i].uAIState != Dying &&
                    pActors[i].uAIState != Removed &&
                    pActors[i].uAIState != Disabled &&
                    pActors[i].uAIState != Summoned &&
                    (pActors[i].ActorEnemy() ||
                     pActors[i].GetActorsRelation(0)))
                    return true;
            }
        }
    }
    return false;
}

//----- (00426A5A) --------------------------------------------------------
void Actor::LootActor() {
    signed int v2;       // edi@1
    unsigned __int8 v7;  // al@30
    char *v9;            // [sp-4h] [bp-3Ch]@10
    char *v10;           // [sp-4h] [bp-3Ch]@31
                         //  char *v11; // [sp-4h] [bp-3Ch]@38
    ItemGen Dst;         // [sp+Ch] [bp-2Ch]@1
    bool itemFound;      // [sp+30h] [bp-8h]@1
    int v14;             // [sp+34h] [bp-4h]@1

    pParty->sub_421B2C_PlaceInInventory_or_DropPickedItem();
    Dst.Reset();
    v2 = 0;
    itemFound = false;
    v14 = 0;
    if (!ActorHasItem()) {
        for (uchar i = 0; i < this->pMonsterInfo.uTreasureDiceRolls; i++)
            v14 += rand() % this->pMonsterInfo.uTreasureDiceSides + 1;
        if (v14) {
            pParty->PartyFindsGold(v14, 0);
            viewparams->bRedrawGameUI = 1;
        }
    } else {
        if (this->ActorHasItems[3].uItemID != 0 &&
            this->ActorHasItems[3].GetItemEquipType() == EQUIP_GOLD) {
            v14 = this->ActorHasItems[3].special_enchantment;
            this->ActorHasItems[3].Reset();
            if (v14) {
                pParty->PartyFindsGold(v14, 0);
                viewparams->bRedrawGameUI = 1;
            }
        }
    }
    if (this->uCarriedItemID) {
        Dst.Reset();
        Dst.uItemID = this->uCarriedItemID;
        v9 = pItemsTable->pItems[Dst.uItemID].pUnidentifiedName;

        if (v14)
            GameUI_StatusBar_OnEvent(localization->FormatString(490, v14, v9));
        else
            GameUI_StatusBar_OnEvent(localization->FormatString(471, v9));

        if (Dst.GetItemEquipType() == 12) {
            Dst.uNumCharges = rand() % 6 + Dst.GetDamageMod() + 1;
            Dst.uMaxCharges = Dst.uNumCharges;
        }
        if (pItemsTable->pItems[Dst.uItemID].uEquipType == 14 &&
            Dst.uItemID != 220)
            Dst.uEnchantmentType = 2 * rand() % 4 + 2;
        pItemsTable->SetSpecialBonus(&Dst);
        if (!pParty->AddItemToParty(&Dst)) pParty->SetHoldingItem(&Dst);
        this->uCarriedItemID = 0;
        if (this->ActorHasItems[0].uItemID) {
            if (!pParty->AddItemToParty(this->ActorHasItems)) {
                pParty->sub_421B2C_PlaceInInventory_or_DropPickedItem();
                pParty->SetHoldingItem(this->ActorHasItems);
            }
            this->ActorHasItems[0].Reset();
        }
        if (this->ActorHasItems[1].uItemID) {
            if (!pParty->AddItemToParty(&this->ActorHasItems[1])) {
                pParty->sub_421B2C_PlaceInInventory_or_DropPickedItem();
                pParty->SetHoldingItem(&this->ActorHasItems[1]);
            }
            this->ActorHasItems[1].Reset();
        }
        this->Remove();
        return;
    }
    if (this->ActorHasItem()) {
        if (this->ActorHasItems[3].uItemID) {
            memcpy(&Dst, &this->ActorHasItems[3], sizeof(Dst));
            this->ActorHasItems[3].Reset();
            // v11 = pItemsTable->pItems[Dst.uItemID].pUnidentifiedName;

            if (v14)
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    490, v14,
                    pItemsTable->pItems[Dst.uItemID].pUnidentifiedName));
            else
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    471, pItemsTable->pItems[Dst.uItemID].pUnidentifiedName));

            if (!pParty->AddItemToParty(&Dst)) pParty->SetHoldingItem(&Dst);
            itemFound = true;
        }
    } else {
        if (rand() % 100 < this->pMonsterInfo.uTreasureDropChance &&
            (v7 = this->pMonsterInfo.uTreasureLevel) != 0) {
            pItemsTable->GenerateItem(v7, this->pMonsterInfo.uTreasureType,
                                      &Dst);
            v10 = pItemsTable->pItems[Dst.uItemID].pUnidentifiedName;

            if (v14)
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    490, v14,
                    v10));  // You found %d gold and an item (%s)!   Вы нашли
                            // ^I[%d] золот^L[ой;ых;ых] и предмет (%s)!
            else
                GameUI_StatusBar_OnEvent(localization->FormatString(
                    471, v10));  // You found %s!   Вы нашли ^Pv[%s]!

            if (!pParty->AddItemToParty(&Dst)) pParty->SetHoldingItem(&Dst);
            itemFound = true;
        }
    }
    if (this->ActorHasItems[0].uItemID) {
        if (!pParty->AddItemToParty(this->ActorHasItems)) {
            pParty->sub_421B2C_PlaceInInventory_or_DropPickedItem();
            pParty->SetHoldingItem(this->ActorHasItems);
            itemFound = true;
        }
        this->ActorHasItems[0].Reset();
    }
    if (this->ActorHasItems[1].uItemID) {
        if (!pParty->AddItemToParty(&this->ActorHasItems[1])) {
            pParty->sub_421B2C_PlaceInInventory_or_DropPickedItem();
            pParty->SetHoldingItem(&this->ActorHasItems[1]);
            itemFound = true;
        }
        this->ActorHasItems[1].Reset();
    }
    if (!itemFound || rand() % 100 < 90)  // for repeatedly get gold and item
        this->Remove();
}

//----- (00427102) --------------------------------------------------------
bool Actor::_427102_IsOkToCastSpell(enum SPELL_TYPE spell) {
    switch (spell) {
        case SPELL_BODY_POWER_CURE: {
            if (this->sCurrentHP >= this->pMonsterInfo.uHP) return false;
            return true;
        }

        case SPELL_LIGHT_DISPEL_MAGIC: {
            for (int i = 0; i < 20; i++) {
                if (pParty->pPartyBuffs[i].Active()) return true;
            }
            for (int i = 1; i <= 4; i++) {
                for (int j = 0; j < 22; j++) {
                    if (pPlayers[i]->pPlayerBuffs[j].Active()) return true;
                }
            }
            return false;
        }

        case SPELL_LIGHT_DAY_OF_PROTECTION:
            return this->pActorBuffs[ACTOR_BUFF_DAY_OF_PROTECTION].Expired();
        case SPELL_LIGHT_HOUR_OF_POWER:
            return this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Expired();
        case SPELL_DARK_PAIN_REFLECTION:
            return this->pActorBuffs[ACTOR_BUFF_PAIN_REFLECTION].Expired();
        case SPELL_BODY_HAMMERHANDS:
            return this->pActorBuffs[ACTOR_BUFF_PAIN_HAMMERHANDS].Expired();
        case SPELL_FIRE_HASTE:
            return this->pActorBuffs[ACTOR_BUFF_HASTE].Expired();
        case SPELL_AIR_SHIELD:
            return this->pActorBuffs[ACTOR_BUFF_SHIELD].Expired();
        case SPELL_EARTH_STONESKIN:
            return this->pActorBuffs[ACTOR_BUFF_STONESKIN].Expired();
        case SPELL_SPIRIT_BLESS:
            return this->pActorBuffs[ACTOR_BUFF_BLESS].Expired();
        case SPELL_SPIRIT_FATE:
            return this->pActorBuffs[ACTOR_BUFF_FATE].Expired();
        case SPELL_SPIRIT_HEROISM:
            return this->pActorBuffs[ACTOR_BUFF_HEROISM].Expired();
        default:
            return true;
    }
}

//----- (0042704B) --------------------------------------------------------
ABILITY_INDEX Actor::special_ability_use_check(int a2) {
    signed int okToCastSpell1;  // ebx@5
    signed int okToCastSpell2;  // edi@5

    if (this->pMonsterInfo.uSpecialAbilityType == 2 &&
        this->pMonsterInfo.uSpecialAbilityDamageDiceBonus < 3 &&
        rand() % 100 < 5)
        this->SummonMinion(a2);
    okToCastSpell1 =
        this->_427102_IsOkToCastSpell((SPELL_TYPE)this->pMonsterInfo.uSpell1ID);
    okToCastSpell2 =
        this->_427102_IsOkToCastSpell((SPELL_TYPE)this->pMonsterInfo.uSpell2ID);
    if (okToCastSpell1 && this->pMonsterInfo.uSpell1UseChance &&
        rand() % 100 < this->pMonsterInfo.uSpell1UseChance)
        return ABILITY_SPELL1;
    if (okToCastSpell2 && this->pMonsterInfo.uSpell2UseChance &&
        rand() % 100 < this->pMonsterInfo.uSpell2UseChance)
        return ABILITY_SPELL2;
    if (this->pMonsterInfo.uAttack2Chance &&
        rand() % 100 < this->pMonsterInfo.uAttack2Chance)
        return ABILITY_ATTACK2;
    return ABILITY_ATTACK1;
}

//----- (004273BB) --------------------------------------------------------
bool Actor::_4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4) {
    signed int v6;   // ebx@1
    signed int v7;   // esi@1
    int armorSum;    // ebx@10
    signed int a2a;  // [sp+18h] [bp+Ch]@1

    v6 = defender->pMonsterInfo.uAC;
    v7 = 0;
    a2a = 0;
    if (defender->pActorBuffs[ACTOR_BUFF_SOMETHING_THAT_HALVES_AC].Active())
        v6 /= 2;
    if (defender->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v7 = defender->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].uPower;
    if (defender->pActorBuffs[ACTOR_BUFF_STONESKIN].Active() &&
        defender->pActorBuffs[ACTOR_BUFF_STONESKIN].uPower > v7)
        v7 = defender->pActorBuffs[ACTOR_BUFF_STONESKIN].uPower;
    armorSum = v7 + v6;
    if (this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        a2a = this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].uPower;
    if (this->pActorBuffs[ACTOR_BUFF_BLESS].Active() &&
        this->pActorBuffs[ACTOR_BUFF_BLESS].uPower > a2a)
        a2a = this->pActorBuffs[ACTOR_BUFF_BLESS].uPower;
    if (this->pActorBuffs[ACTOR_BUFF_FATE].Active()) {
        a2a += this->pActorBuffs[ACTOR_BUFF_FATE].uPower;
        this->pActorBuffs[ACTOR_BUFF_FATE].Reset();
    }
    return rand() % (armorSum + 2 * this->pMonsterInfo.uLevel + 10) + a2a + 1 >
           armorSum + 5;
}

//----- (004274AD) --------------------------------------------------------
bool Actor::ActorHitOrMiss(Player *pPlayer) {
    signed int v3;  // edi@1
    signed int v4;  // esi@8
    int v5;         // esi@8

    v3 = 0;
    if (this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v3 = this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].uPower;
    if (this->pActorBuffs[ACTOR_BUFF_BLESS].Active() &&
        this->pActorBuffs[ACTOR_BUFF_BLESS].uPower > v3)
        v3 = this->pActorBuffs[ACTOR_BUFF_BLESS].uPower;
    if (this->pActorBuffs[ACTOR_BUFF_FATE].Active()) {
        v3 += this->pActorBuffs[ACTOR_BUFF_FATE].uPower;
        this->pActorBuffs[ACTOR_BUFF_FATE].Reset();
    }
    v4 = pPlayer->GetActualAC() + 2 * this->pMonsterInfo.uLevel + 10;
    v5 = rand() % v4 + 1;
    return (v3 + v5 > pPlayer->GetActualAC() + 5);
}

//----- (0042756B) --------------------------------------------------------
int Actor::CalcMagicalDamageToActor(DAMAGE_TYPE dmgType,
                                    signed int incomingDmg) {
    int v4;             // edx@1
    int v5;             // ecx@1
    signed int v6;      // eax@4
    signed int result;  // eax@17
    signed int v8;      // esi@18

    v4 = 0;
    v5 = 0;
    if (this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v5 = this->pActorBuffs[ACTOR_BUFF_HOUR_OF_POWER].uPower;
    switch (dmgType) {
        case DMGT_FIRE:
            v6 = this->pMonsterInfo.uResFire;
            v4 = v5;
            break;
        case DMGT_ELECTR:
            v6 = this->pMonsterInfo.uResAir;
            v4 = v5;
            break;
        case DMGT_COLD:
            v6 = this->pMonsterInfo.uResWater;
            v4 = v5;
            break;
        case DMGT_EARTH:
            v6 = this->pMonsterInfo.uResEarth;
            v4 = v5;
            break;
        case DMGT_PHISYCAL:
            v6 = this->pMonsterInfo.uResPhysical;
            break;
        case DMGT_SPIRIT:
            v6 = this->pMonsterInfo.uResSpirit;
            break;
        case DMGT_MIND:
            v6 = this->pMonsterInfo.uResMind;
            v4 = v5;
            break;
        case DMGT_BODY:
            v6 = this->pMonsterInfo.uResBody;
            v4 = v5;
            break;
        case DMGT_LIGHT:
            v6 = this->pMonsterInfo.uResLight;
            break;
        case DMGT_DARK:
            v6 = this->pMonsterInfo.uResDark;
            break;
        default:
            v6 = 0;
            break;
    }
    if (v6 < 200) {
        v8 = v4 + v6 + 30;
        for (int i = 0; i < 4; i++) {
            if (rand() % v8 < 30) break;
            incomingDmg /= 2;
        }
        result = incomingDmg;
    } else {
        result = 0;
    }
    return result;
}

//----- (00427662) --------------------------------------------------------
bool Actor::DoesDmgTypeDoDamage(DAMAGE_TYPE uType) {
    signed int resist;  // esi@2
    bool result;        // eax@13

    switch (uType) {
        case 0:
            resist = this->pMonsterInfo.uResFire;
            break;
        case 1:
            resist = this->pMonsterInfo.uResAir;
            break;
        case 2:
            resist = this->pMonsterInfo.uResWater;
            break;
        case 3:
            resist = this->pMonsterInfo.uResEarth;
            break;
        case 4:
            resist = this->pMonsterInfo.uResPhysical;
            break;
        case 6:
            resist = this->pMonsterInfo.uResSpirit;
            break;
        case 7:
            resist = this->pMonsterInfo.uResMind;
        case 8:
            resist = this->pMonsterInfo.uResBody;
            break;
        case 9:
            resist = this->pMonsterInfo.uResLight;
            break;
        case 10:
            resist = this->pMonsterInfo.uResDark;
            break;
        default:
            return 1;
    }
    if (resist < 200)
        result = rand() % ((this->pMonsterInfo.uLevel >> 2) + resist + 30) < 30;
    else
        result = 0;
    return result;
}

//----- (00448A98) --------------------------------------------------------
void ToggleActorGroupFlag(unsigned int uGroupID, unsigned int uFlag,
                          unsigned int bToggle) {
    if (uGroupID) {
        if (bToggle) {
            for (uint i = 0; i < (unsigned int)uNumActors; ++i) {
                if (pActors[i].uGroup == uGroupID) {
                    pActors[i].uAttributes |= uFlag;
                    if (uFlag == 0x10000) {
                        pActors[i].uAIState = Disabled;
                        pActors[i].UpdateAnimation();
                    }
                }
            }
        } else {
            for (uint i = 0; i < (unsigned int)uNumActors; ++i) {
                if (pActors[i].uGroup == uGroupID) {
                    if (uFlag == 0x10000) {
                        if (pActors[i].uAIState != Dead) {
                            if (pActors[i].uAIState != 4 &&
                                pActors[i].uAIState != 11)
                                pActors[i].uAIState = Standing;
                        }
                    }
                    HEXRAYS_LODWORD(pActors[i].uAttributes) &= ~uFlag;
                }
            }
        }
    }
}

//----- (004014E6) --------------------------------------------------------
void Actor::MakeActorAIList_ODM() {
    int v1;           // eax@4
    unsigned int v7;  // ST20_4@10
    int distance;     // edi@10
                      //  int v10; // ebx@14
    int v21;          // [sp+Ch] [bp-14h]@4
    int v22;          // [sp+10h] [bp-10h]@4

    pParty->uFlags &= 0xFFFFFFCF;  // ~0x30

    ai_arrays_size = 0;
    for (uint i = 0; i < uNumActors; ++i) {
        Actor *actor = &pActors[i];

        actor->ResetAlive();  // ~0x400
        if (!actor->CanAct()) {
            actor->ResetActive();
            continue;
        }

        v22 = abs(pParty->vPosition.z - actor->vPosition.z);
        v21 = abs(pParty->vPosition.y - actor->vPosition.y);
        v1 = abs(pParty->vPosition.x - actor->vPosition.x);
        v7 = int_get_vector_length(v22, v21, v1);
        distance = v7 - actor->uActorRadius;
        if (distance < 0) distance = 0;

        if (distance < 5632) {
            actor->ResetHostile();
            if (actor->ActorEnemy() || actor->GetActorsRelation(0)) {
                // v11 = (pParty->uFlags & 0x10) == 0;
                actor->uAttributes |= ACTOR_HOSTILE;
                if (distance < 5120) pParty->SetYellowAlert();
                if (distance < 307) pParty->SetRedAlert();
            }
            actor->uAttributes |= ACTOR_ACTIVE;
            ai_near_actors_distances[ai_arrays_size] = distance;
            ai_near_actors_ids[ai_arrays_size++] = i;
        } else {
            actor->ResetActive();
        }
    }

    /*
    result = v27;
    if ( v27 > 0 )
    {
      v14 = 0;
      v15 = 1;
      v26 = 1;
      do
      {
        while ( 1 )
        {
          v24 = v15;
          if ( v15 >= result )
            break;
          v16 = ai_near_actors_distances[v14];
          if ( v16 > ai_near_actors_distances[v15] )
          {
            v17 = &ai_near_actors_ids[v15];
            v18 = ai_near_actors_ids[v14];
            ai_near_actors_ids[v14] = *v17;
            *v17 = v18;
            v15 = v24;
            ai_near_actors_distances[v14] = ai_near_actors_distances[v24];
            ai_near_actors_distances[v24] = v16;
          }
          result = v27;
          ++v15;
        }
        ++v14;
        v15 = v26 + 1;
        v26 = v15;
      }
      while ( v15 - 1 < result );
    }*/

    for (int i = 0; i < ai_arrays_size; ++i) {
        for (int j = 0; j < i; ++j) {
            if (ai_near_actors_distances[j] > ai_near_actors_distances[i]) {
                int tmp = ai_near_actors_distances[j];
                ai_near_actors_distances[j] = ai_near_actors_distances[i];
                ai_near_actors_distances[i] = tmp;

                tmp = ai_near_actors_ids[j];
                ai_near_actors_ids[j] = ai_near_actors_ids[i];
                ai_near_actors_ids[i] = tmp;
            }
        }
    }

    if (ai_arrays_size > 30) ai_arrays_size = 30;

    for (int i = 0; i < ai_arrays_size; ++i)
        pActors[ai_near_actors_ids[i]].uAttributes |= ACTOR_ALIVE;  // 0x400
}

//----- (004016FA) --------------------------------------------------------
int Actor::MakeActorAIList_BLV() {
    int v1;            // eax@4
    int distance;      // edi@10
    int v13;           // edx@24
    int v15;           // ebx@26
    unsigned int v17;  // esi@27
    int v18;           // ecx@31
    signed int v19;    // edi@31
    signed int v25;    // eax@40
    uint j;             // edi@45
    int v30;           // eax@48
    int v37;           // [sp+Ch] [bp-18h]@1
    int v38;           // [sp+10h] [bp-14h]@4
    int v39;           // [sp+14h] [bp-10h]@4
    uint i;             // [sp+18h] [bp-Ch]@31
    uint v45;          // [sp+20h] [bp-4h]@1

    //  __debugbreak(); // refactor for blv ai
    pParty->uFlags &= 0xFFFFFFCF;  // ~0x30
    v37 = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                             pParty->vPosition.z);
    v45 = 0;
    for (uint i = 0; i < uNumActors; ++i) {
        pActors[i].ResetAlive();  // ~0x0400
        if (!pActors[i].CanAct()) {
            pActors[i].ResetActive();
            continue;
        }
        v1 = abs(pParty->vPosition.x - pActors[i].vPosition.x);
        v38 = abs(pParty->vPosition.y - pActors[i].vPosition.y);
        v39 = abs(pParty->vPosition.z - pActors[i].vPosition.z);

        distance =
            int_get_vector_length(v39, v38, v1) - pActors[i].uActorRadius;
        if (distance < 0) distance = 0;
        if (distance < 10240) {
            pActors[i].ResetHostile();  // ~0x01000000
            if (pActors[i].ActorEnemy() || pActors[i].GetActorsRelation(0)) {
                pActors[i].uAttributes |= ACTOR_HOSTILE;
                if (!(pParty->uFlags & 0x10) && (double)distance < 307.2)
                    pParty->SetRedAlert();
                if (!(pParty->uFlags & 0x20) && distance < 5120)
                    pParty->SetYellowAlert();
            }
            ai_near_actors_distances[v45] = distance;
            ai_near_actors_ids[v45] = i;
            v45++;
        } else {
            pActors[i].ResetActive();
        }
    }
    v13 = 0;
    if (v45 > 0) {
        for (uint i = 1; i < v45; i++) {
            for (uint j = 1; j < v45; ++j) {
                v15 = ai_near_actors_distances[v13];
                if (ai_near_actors_distances[v13] >
                    ai_near_actors_distances[j]) {
                    v17 = ai_near_actors_ids[v13];
                    ai_near_actors_ids[v13] = ai_near_actors_ids[j];
                    ai_near_actors_ids[j] = v17;
                    ai_near_actors_distances[v13] = ai_near_actors_distances[j];
                    ai_near_actors_distances[j] = v15;
                }
            }
            ++v13;
        }
    }
    v18 = 0;
    v19 = 0;
    for (i = 0; i < v45; i++) {
        if (pActors[ai_near_actors_ids[i]].ActorNearby() ||
            sub_4070EF_prolly_detect_player(
                PID(OBJECT_Actor, ai_near_actors_ids[i]), 4)) {
            pActors[ai_near_actors_ids[i]].uAttributes |= ACTOR_NEARBY;
            ai_array_4F6638_actor_ids[v19] = ai_near_actors_ids[i];
            ai_array_4F5E68[v19++] = ai_near_actors_distances[i];
            if (v19 >= 30) break;
        }
    }
    ai_arrays_size = v19;
    if ((signed int)uNumActors > 0) {
        for (uint i = 0; i < (signed int)uNumActors; ++i) {
            if (pActors[i].CanAct() && pActors[i].uSectorID == v37) {
                v25 = 0;
                if (v19 <= 0) {
                    pActors[i].uAttributes |= ACTOR_ACTIVE;
                    ai_array_4F6638_actor_ids[ai_arrays_size++] = i;
                } else {
                    while (ai_array_4F6638_actor_ids[v25] != i) {
                        ++v25;
                        if (v25 >= v19) {
                            pActors[i].uAttributes |= ACTOR_ACTIVE;
                            ai_array_4F6638_actor_ids[ai_arrays_size++] = i;
                            break;
                        }
                    }
                }
            }
        }
    }
    for (j = 0; j < v45; ++j) {
        if (pActors[ai_near_actors_ids[j]].uAttributes & 0xC000 &&
            pActors[ai_near_actors_ids[j]].CanAct()) {
            v30 = 0;
            if (ai_arrays_size <= 0) {
                ai_array_4F6638_actor_ids[ai_arrays_size++] =
                    ai_near_actors_ids[j];
            } else {
                while (ai_array_4F6638_actor_ids[v30] !=
                       ai_near_actors_ids[j]) {
                    ++v30;
                    if (v30 >= ai_arrays_size) {
                        ai_array_4F6638_actor_ids[ai_arrays_size++] =
                            ai_near_actors_ids[j];
                        break;
                    }
                }
            }
        }
    }
    if (ai_arrays_size > 30) ai_arrays_size = 30;
    memcpy(ai_near_actors_ids.data(), ai_array_4F6638_actor_ids.data(),
           4 * ai_arrays_size);
    memcpy(ai_near_actors_distances.data(), ai_array_4F5E68.data(),
           4 * ai_arrays_size);
    for (int i = 0; i < ai_arrays_size; i++)
        pActors[ai_near_actors_ids[i]].uAttributes |= ACTOR_ALIVE;  // 0x400
    return ai_arrays_size;
}

//----- (004070EF) --------------------------------------------------------
bool sub_4070EF_prolly_detect_player(unsigned int uObjID,
                                     unsigned int uObj2ID) {
    signed int v2;               // eax@1
    int obj1_sector;             // eax@4
    float v8;                    // ST24_4@5
    signed int v12;              // eax@7
    int obj2_z;                  // edi@11
    int obj2_x;                  // esi@11
    int obj2_sector;             // eax@13
    float v20;                   // ST24_4@14
    int dist_x;                  // ebx@16
    signed int dist_3d;          // ecx@16
    int v25;                     // eax@18
    BLVFace *v29;                // ebx@32
    Vec3_short_ *v30;            // esi@32
    int v31;                     // eax@32
    int v32;                     // ST50_4@44
    int v33;                     // ST54_4@44
    int v34;                     // eax@44
    int v38;              // esi@45
    __int16 next_sector;         // bx@58
    int v47;                     // [sp+18h] [bp-50h]@20
    int v48;                     // [sp+1Ch] [bp-4Ch]@20
    int v49;                     // [sp+20h] [bp-48h]@20
    int dist_z;                  // [sp+24h] [bp-44h]@16
    int higher_z;         // [sp+24h] [bp-44h]@27
    int lower_z;          // [sp+28h] [bp-40h]@26
    int higher_y;         // [sp+2Ch] [bp-3Ch]@23
    int lower_y;          // [sp+30h] [bp-38h]@22
    int higher_x;         // [sp+34h] [bp-34h]@21
    int lower_x;          // [sp+38h] [bp-30h]@20
    int sectors_visited;  // [sp+3Ch] [bp-2Ch]@28
    int v58;                     // [sp+44h] [bp-24h]@50
    int v59;                     // [sp+48h] [bp-20h]@44
    int obj2_y;                  // [sp+50h] [bp-18h]@11
    int obj1_x;                  // [sp+58h] [bp-10h]@4
    int obj1_y;                  // [sp+5Ch] [bp-Ch]@4
    int obj1_z;                  // [sp+60h] [bp-8h]@4
    int current_sector;          // [sp+64h] [bp-4h]@7
    int dist_y;
    int v70;

    v2 = PID_ID(uObjID);
    switch (PID_TYPE(uObjID)) {
        case OBJECT_Decoration:
            obj1_x = pLevelDecorations[v2].vPosition.x;
            obj1_y = pLevelDecorations[v2].vPosition.y;
            obj1_z = pLevelDecorations[v2].vPosition.z;
            obj1_sector = pIndoor->GetSector(obj1_x, obj1_y, obj1_z);
            break;
        case OBJECT_Actor:
            obj1_x = pActors[v2].vPosition.x;
            obj1_y = pActors[v2].vPosition.y;
            v8 = (float)pActors[v2].uActorHeight * 0.69999999;
            // v9 = v8 + 6.7553994e15;
            // obj1_z = LODWORD(v9) + pActors[v2].vPosition.z;
            obj1_z = (int)v8 + pActors[v2].vPosition.z;
            obj1_sector = pActors[v2].uSectorID;
            break;
        case OBJECT_Item:
            obj1_x = pSpriteObjects[v2].vPosition.x;
            obj1_y = pSpriteObjects[v2].vPosition.y;
            obj1_z = pSpriteObjects[v2].vPosition.z;
            obj1_sector = pSpriteObjects[v2].uSectorID;
            break;
        default:
            return 0;
    }
    v12 = PID_ID(uObj2ID);
    switch (PID_TYPE(uObj2ID)) {
        case OBJECT_Decoration:
            obj2_z = pLevelDecorations[v12].vPosition.z;
            obj2_x = pLevelDecorations[v12].vPosition.x;
            obj2_y = pLevelDecorations[v12].vPosition.y;
            obj2_sector = pIndoor->GetSector(obj2_x, obj2_y, obj2_z);
            break;
        case OBJECT_Player:
            obj2_x = pParty->vPosition.x;
            obj2_z = pParty->sEyelevel + pParty->vPosition.z;
            obj2_y = pParty->vPosition.y;
            obj2_sector =
                pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                   pParty->sEyelevel + pParty->vPosition.z);
            break;
        case OBJECT_Actor:
            obj2_y = pActors[v12].vPosition.y;
            obj2_x = pActors[v12].vPosition.x;
            v20 = (float)pActors[v12].uActorHeight * 0.69999999;
            // v21 = v20 + 6.7553994e15;
            // obj2_z = LODWORD(v21) + pActors[v12].vPosition.z;
            obj2_z = (int)v20 + pActors[v12].vPosition.z;
            obj2_sector = pActors[v12].uSectorID;
            break;
        case OBJECT_Item:
            obj2_x = pSpriteObjects[v12].vPosition.x;
            obj2_z = pSpriteObjects[v12].vPosition.z;
            obj2_y = pSpriteObjects[v12].vPosition.y;
            obj2_sector = pSpriteObjects[v12].uSectorID;
            break;
        default:
            return 0;
    }
    dist_x = obj2_x - obj1_x;
    dist_z = obj2_z - obj1_z;
    dist_y = obj2_y - obj1_y;
    dist_3d = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
    // range check
    if (dist_3d > 5120) return 0;
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) return 1;
    v25 = 65536;
    if (dist_3d) v25 = 65536 / dist_3d;
    v49 = dist_x * v25;
    v47 = dist_z * v25;
    v48 = dist_y * v25;
    if (obj1_x < obj2_x) {
        lower_x = obj1_x;
        higher_x = obj2_x;
    } else {
        lower_x = obj2_x;
        higher_x = obj1_x;
    }
    if (obj1_y < obj2_y) {
        lower_y = obj1_y;
        higher_y = obj2_y;
    } else {
        lower_y = obj2_y;
        higher_y = obj1_y;
    }
    if (obj1_z < obj2_z) {
        lower_z = obj1_z;
        higher_z = obj2_z;
    } else {
        lower_z = obj2_z;
        higher_z = obj1_z;
    }
    sectors_visited = 0;
    // monster in same sector with player
    if (obj1_sector == obj2_sector) return 1;
    // search starts from monster
    current_sector = obj1_sector;
    for (int current_portal = 0;
         current_portal < pIndoor->pSectors[current_sector].uNumPortals;
         current_portal++) {
        v29 = &pIndoor->pFaces[pIndoor->pSectors[current_sector]
                                   .pPortals[current_portal]];
        v30 = &pIndoor->pVertices[*v29->pVertexIDs];
        v31 = v29->pFacePlane_old.vNormal.z * (v30->z - obj1_z) +
              v29->pFacePlane_old.vNormal.y * (v30->y - obj1_y) +
              v29->pFacePlane_old.vNormal.x * (v30->x - obj1_x);

        if (current_sector != v29->uSectorID) v31 = -v31;

        if (v31 >= 0 && v30->x != obj1_x && v30->y != obj1_y &&
            v30->z != obj1_z)
            continue;

        if (lower_x > v29->pBounding.x2 || higher_x < v29->pBounding.x1 ||
            lower_y > v29->pBounding.y2 || higher_y < v29->pBounding.y1 ||
            lower_z > v29->pBounding.z2 || higher_z < v29->pBounding.z1) {
            continue;
        }

        v32 = fixpoint_mul(v29->pFacePlane_old.vNormal.x, v49);
        v34 = fixpoint_mul(v29->pFacePlane_old.vNormal.y, v48);
        v33 = fixpoint_mul(v29->pFacePlane_old.vNormal.z, v47);

        v59 = v32 + v33 + v34;
        if (v59) {
            v70 = v29->pFacePlane_old.dist +
                  obj1_z * v29->pFacePlane_old.vNormal.z +
                  obj1_x * v29->pFacePlane_old.vNormal.x +
                  obj1_y * v29->pFacePlane_old.vNormal.y;
            v38 = -v70;

            // if ( v59 <= 0 ^ v70 <= 0 )

            /* TEMPORARY
            if ( v59 <= 0 && v70 <= 0 )
            {
                    continue;
            }
            if ( !(v59 <= 0 && v70 <= 0) )
            {
                    continue;
            }
            */

            if (abs(v38) >> 14 > abs(v59)) continue;

            v58 = fixpoint_div(v38, v59);

            if (v58 < 0) continue;

            if (!sub_4075DB(obj1_x + ((fixpoint_mul(v49, v58) + 32768) >> 16),
                            obj1_y + ((fixpoint_mul(v48, v58) + 32768) >> 16),
                            obj1_z + ((fixpoint_mul(v47, v58) + 32768) >> 16),
                            v29)) {
                continue;
            }

            // if there is no next sector turn back
            if (v29->uSectorID == current_sector)
                next_sector = v29->uBackSectorID;
            else
                next_sector = v29->uSectorID;

            // no more portals, quit
            if (next_sector == current_sector) break;

            ++sectors_visited;
            current_sector = next_sector;

            // found player, quit
            if (next_sector == obj2_sector) return 1;

            current_sector = next_sector;

            // did we hit limit for portals?
            // does the next room have portals?
            if (sectors_visited < 30 &&
                pIndoor->pSectors[current_sector].uNumPortals > 0) {
                current_portal = -1;
                continue;
            } else {
                break;
            }
        }
    }
    // did we stop in the sector where player is?
    if (current_sector != obj2_sector) return 0;
    return 1;
}

//----- (00450B0A) --------------------------------------------------------
bool SpawnActor(unsigned int uMonsterID) {
    unsigned int v1;  // ebx@1
    bool result;      // eax@2
    unsigned int v6;  // ecx@5
    Actor actor;      // [sp+4h] [bp-350h]@5
    Vec3_int_ pOut;   // [sp+348h] [bp-Ch]@5

    v1 = uMonsterID;
    if (uNumActors == 499) {
        result = 0;
    } else {
        if ((signed int)uMonsterID >= (signed int)pMonsterList->uNumMonsters)
            v1 = 0;
        memset(&actor, 0, sizeof(Actor));
        strcpy(actor.pActorName, pMonsterStats->pInfos[v1 + 1].pName);
        actor.sCurrentHP = (short)pMonsterStats->pInfos[v1 + 1].uHP;
        memcpy(&actor.pMonsterInfo, &pMonsterStats->pInfos[v1 + 1],
               sizeof(MonsterInfo));
        actor.word_000086_some_monster_id = v1 + 1;
        actor.uActorRadius = pMonsterList->pMonsters[v1].uMonsterRadius;
        actor.uActorHeight = pMonsterList->pMonsters[v1].uMonsterHeight;
        actor.uMovementSpeed = pMonsterList->pMonsters[v1].uMovementSpeed;

        Vec3_int_::Rotate(200, pParty->sRotationY, 0, pParty->vPosition,
                          &pOut.x, &pOut.z, &pOut.y);
        actor.vInitialPosition.x = pOut.x;
        actor.vPosition.x = pOut.x;
        actor.uTetherDistance = 256;
        actor.vInitialPosition.y = (short)pOut.z;
        actor.vPosition.y = (short)pOut.z;
        actor.vInitialPosition.z = (short)pOut.y;
        actor.vPosition.z = (short)pOut.y;
        pSprites_LOD->DeleteSomeSprites();
        pPaletteManager->ResetNonTestLocked();
        v6 = uNumActors - 1;
        if (dword_5C6DF8 == 1) {
            dword_5C6DF8 = 0;
            v6 = uNumActors++;
        }
        memcpy(&pActors[v6], &actor, sizeof(Actor));
        pActors[v6].PrepareSprites(1);
        result = 1;
    }
    return result;
}

//----- (0044FA4C) --------------------------------------------------------
int sub_44FA4C_spawn_light_elemental(int spell_power, int caster_skill_level,
                                     int duration_game_seconds) {
    int result;     // eax@13
    int v10;               // ebx@16
    const char *v15;       // [sp-4h] [bp-24h]@2
    unsigned int uFaceID;  // [sp+8h] [bp-18h]@16
    int v19;               // [sp+Ch] [bp-14h]@16
    size_t v20;            // [sp+10h] [bp-10h]@6
    int v21;               // [sp+14h] [bp-Ch]@14
    unsigned int v23;      // [sp+1Ch] [bp-4h]@6

    if (caster_skill_level == 4)
        v15 = "Elemental Light C";
    else if (caster_skill_level == 3)
        v15 = "Elemental Light B";
    else
        v15 = "Elemental Light A";

    v23 = pMonsterList->GetMonsterIDByName(v15);
    v20 = 0;
    for (v20; v20 < uNumActors; v20++) {
        if (pActors[v20].uAIState == Removed) break;
    }

    result = uNumActors + 1;
    if (v20 != uNumActors || result < 500) {
        v21 = 0;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
            v21 = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                     pParty->vPosition.z);
        v19 = (((uCurrentlyLoadedLevelType != LEVEL_Outdoor) - 1) & 0x40) + 64;
        pActors[v20].Reset();
        strcpy(pActors[v20].pActorName, pMonsterStats->pInfos[v23 + 1].pName);
        pActors[v20].sCurrentHP = pMonsterStats->pInfos[v23 + 1].uHP;
        memcpy(&pActors[v20].pMonsterInfo, &pMonsterStats->pInfos[v23 + 1],
               sizeof(MonsterInfo));
        pActors[v20].word_000086_some_monster_id = v23 + 1;
        pActors[v20].uActorRadius = pMonsterList->pMonsters[v23].uMonsterRadius;
        pActors[v20].uActorHeight = pMonsterList->pMonsters[v23].uMonsterHeight;
        pActors[v20].pMonsterInfo.uTreasureDiceRolls = 0;
        pActors[v20].pMonsterInfo.uTreasureType = 0;
        pActors[v20].pMonsterInfo.uExp = 0;
        pActors[v20].uMovementSpeed =
            pMonsterList->pMonsters[v23].uMovementSpeed;
        v10 = rand() % 2048;
        pActors[v20].vInitialPosition.x =
            pParty->vPosition.x + fixpoint_mul(stru_5C6E00->Cos(v10), v19);
        pActors[v20].vPosition.x = pActors[v20].vInitialPosition.x;
        pActors[v20].vInitialPosition.y =
            pParty->vPosition.y + fixpoint_mul(stru_5C6E00->Sin(v10), v19);
        pActors[v20].vPosition.y = pActors[v20].vInitialPosition.y;
        pActors[v20].vInitialPosition.z = pParty->vPosition.z;
        pActors[v20].vPosition.z = pActors[v20].vInitialPosition.z;
        pActors[v20].uTetherDistance = 256;
        pActors[v20].uSectorID = v21;
        pActors[v20].PrepareSprites(0);
        pActors[v20].pMonsterInfo.uHostilityType =
            MonsterInfo::Hostility_Friendly;
        pActors[v20].uAlly = 9999;
        pActors[v20].uGroup = 0;
        pActors[v20].uCurrentActionTime = 0;
        pActors[v20].uAIState = Summoned;
        pActors[v20].uCurrentActionLength = 256;
        pActors[v20].UpdateAnimation();

        result = pIndoor->GetSector(pActors[v20].vPosition.x,
                                    pActors[v20].vPosition.y,
                                    pActors[v20].vPosition.z);
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor ||
            result == v21 &&
                (result = BLV_GetFloorLevel(
                     pActors[v20].vPosition.x, pActors[v20].vPosition.y,
                     pActors[v20].vPosition.z, result, &uFaceID),
                 result != -30000) &&
                (result = abs(result - pParty->vPosition.z), result <= 1024)) {
            if (v20 == uNumActors) ++uNumActors;
            pActors[v20].uSummonerID = PID(OBJECT_Player, spell_power);
            result = pActors[v20].pActorBuffs[ACTOR_BUFF_SUMMONED].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(duration_game_seconds)),
                caster_skill_level, spell_power, 0, 0);
        }
    }
    return result;
}

//----- (0044F57C) --------------------------------------------------------
void SpawnEncounter(MapInfo *pMapInfo, SpawnPointMM7 *spawn, int a3, int a4, int a5) {
    // a3 for abc modify
    // a4 count

    int v7;                // eax@2
    char v8;               // zf@5
    int v12;               // edx@9
    // int v18;               // esi@31
    Actor *pMonster;       // esi@35
    int v23;               // edx@36
    int v24;        // edi@36
    int v25;               // ecx@36
    MonsterDesc *v27;      // edi@48
    signed int v28;        // eax@48
    int v32;               // eax@50
    int v37;               // eax@51
    int v38;               // eax@52
    int v39;               // edi@52
    String v40;       // [sp-18h] [bp-100h]@60
    const char *v44;       // [sp-8h] [bp-F0h]@13
    char *pTexture;        // [sp-4h] [bp-ECh]@9
                           //  char Str[32]; // [sp+Ch] [bp-DCh]@60
    char Str2[120];        // [sp+2Ch] [bp-BCh]@29
    unsigned int uFaceID;  // [sp+A4h] [bp-44h]@52
    MonsterInfo *Src;      // [sp+A8h] [bp-40h]@50
    int v50;               // [sp+ACh] [bp-3Ch]@47
    char Source[32];       // [sp+B0h] [bp-38h]@20
    int v52;               // [sp+D0h] [bp-18h]@34
    int v53;               // [sp+D4h] [bp-14h]@34
    int pSector;           // [sp+D8h] [bp-10h]@32
    int pPosX;             // [sp+DCh] [bp-Ch]@32
    int NumToSpawn;               // [sp+E0h] [bp-8h]@8
    int v57;               // [sp+E4h] [bp-4h]@1

    // auto a2 = spawn;
    v57 = 0;
    // v5 = pMapInfo;
    // v6 = spawn;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        v7 = pOutdoor->ddm.field_C_alert;
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        v7 = pIndoor->dlv.field_C_alert;
    else
        v7 = 0;

    if (v7)
        v8 = (spawn->uAttributes & 1) == 0;
    else
        v8 = (spawn->uAttributes & 1) == 1;
    if (v8) return;

    // result = (void *)(spawn->uIndex - 1);
    NumToSpawn = 1;
    switch (spawn->uIndex - 1) {
        case 0:
            // v9 = pMapInfo->uEncounterMonster1AtLeast;
            // v10 = rand();
            // v11 = pMapInfo->uEncounterMonster1AtMost;
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            v12 = rand() % (pMapInfo->uEncounterMonster1AtMost -
                            pMapInfo->uEncounterMonster1AtLeast + 1);
            // v13 = pMapInfo->Dif_M1;
            v57 = pMapInfo->Dif_M1;
            NumToSpawn = pMapInfo->uEncounterMonster1AtLeast + v12;
            strcpy(Source, pMapInfo->pEncounterMonster1Texture.c_str());
            break;
        case 3:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s A";
            sprintf(Source, "%s A", pMapInfo->pEncounterMonster1Texture.c_str());
            break;
        case 4:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s A";
            sprintf(Source, "%s A", pMapInfo->pEncounterMonster2Texture.c_str());
            break;
        case 5:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s A";
            sprintf(Source, "%s A", pMapInfo->pEncounterMonster3Texture.c_str());
            break;
        case 1:
            // v9 = pMapInfo->uEncounterMonster2AtLeast;
            // v14 = rand();
            // v15 = pMapInfo->uEncounterMonster2AtMost;
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            v12 = rand() % (pMapInfo->uEncounterMonster2AtMost -
                            pMapInfo->uEncounterMonster2AtLeast + 1);
            // v13 = pMapInfo->Dif_M2;
            v57 = pMapInfo->Dif_M2;
            NumToSpawn = pMapInfo->uEncounterMonster2AtLeast + v12;
            strcpy(Source, pMapInfo->pEncounterMonster2Texture.c_str());
            break;
        case 6:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s B";
            sprintf(Source, "%s B", pMapInfo->pEncounterMonster1Texture.c_str());
            break;
        case 7:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s B";
            sprintf(Source, "%s B", pMapInfo->pEncounterMonster2Texture.c_str());
            break;
        case 8:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s B";
            sprintf(Source, "%s B", pMapInfo->pEncounterMonster3Texture.c_str());
            break;
        case 2:
            // v9 = pMapInfo->uEncounterMonster3AtLeast;
            // v16 = rand();
            // v17 = pMapInfo->uEncounterMonster3AtMost;
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            v12 = rand() % (pMapInfo->uEncounterMonster3AtMost -
                            pMapInfo->uEncounterMonster3AtLeast + 1);
            // v13 = pMapInfo->Dif_M3;
            v57 = pMapInfo->Dif_M3;
            NumToSpawn = pMapInfo->uEncounterMonster3AtLeast + v12;
            strcpy(Source, pMapInfo->pEncounterMonster3Texture.c_str());
            break;
        case 9:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s C";
            sprintf(Source, "%s C", pMapInfo->pEncounterMonster1Texture.c_str());
            break;
        case 10:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s C";
            sprintf(Source, "%s C", pMapInfo->pEncounterMonster2Texture.c_str());
            break;
        case 11:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s C";
            sprintf(Source, "%s C", pMapInfo->pEncounterMonster3Texture.c_str());
            break;
        default:
            return;
    }

    if (Source[0] == '0') return;

    v57 += a3;
    if (v57 > 4) v57 = 4;
    strcpy(Str2, Source);
    if (a4) NumToSpawn = a4;
    // v18 = NumToSpawn;
    if (NumToSpawn <= 0) return;
    if ((signed int)(NumToSpawn + uNumActors) >= 500) return;

    pSector = 0;
    pPosX = spawn->vPosition.x;
    a4 = spawn->vPosition.y;
    a3 = spawn->vPosition.z;
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pSector = pIndoor->GetSector(spawn->vPosition.x, spawn->vPosition.y,
                                     spawn->vPosition.z);
    v53 = 0;
    v52 = (((uCurrentlyLoadedLevelType != LEVEL_Outdoor) - 1) & 0x40) + 64;

    // spawning loop
    for (int i = v53; i < NumToSpawn; ++i) {
        pMonster = &pActors[uNumActors];
        pActors[uNumActors].Reset();

        // random monster levels ABC
        if (v57) {
            v23 = rand() % 100;
            v24 = 3;  // 2 , 10 , 20
            v25 = (uint16_t)word_4E8152[3 * v57];
            if (v23 >= v25) {
                if (v23 < v25 + (uint16_t)word_4E8152[3 * v57 + 1]) {
                    v24 = 2;  // 8 , 20 , 30
                }
            } else {
                v24 = 1;  // 90 , 70 , 50
            }

            if (v24 == 1) {
                pTexture = Source;
                v44 = "%s A";
            } else if (v24 == 2) {
                pTexture = Source;
                v44 = "%s B";
            } else {
                if (v24 != 3) continue;
                pTexture = Source;
                v44 = "%s C";
            }

            sprintf(Str2, v44, pTexture);
        }

        v50 = pMonsterList->GetMonsterIDByName(Str2);
        pTexture = Str2;
        if ((signed __int16)v50 == -1) {
            logger->Warning(
                L"Can't create random monster: '%S'! See MapStats.txt and "
                L"Monsters.txt!",
                pTexture);
            Engine_DeinitializeAndTerminate(0);
        }

        v27 = &pMonsterList->pMonsters[(signed __int16)v50];
        v28 = pMonsterStats->FindMonsterByTextureName(pTexture);
        if (!v28) v28 = 1;
        Src = &pMonsterStats->pInfos[v28];
        strcpy(pMonster->pActorName, Src->pName);
        pMonster->sCurrentHP = Src->uHP;
        assert(sizeof(MonsterInfo) == 88);
        memcpy(&pMonster->pMonsterInfo, Src,
               sizeof(MonsterInfo));  // Uninitialized portail memory access
        pMonster->word_000086_some_monster_id = v50 + 1;
        pMonster->uActorRadius = v27->uMonsterRadius;
        pMonster->uActorHeight = v27->uMonsterHeight;
        pMonster->uMovementSpeed = v27->uMovementSpeed;
        pMonster->vInitialPosition.x = spawn->vPosition.x;
        pMonster->vPosition.x = spawn->vPosition.x;
        pMonster->uTetherDistance = 256;
        pMonster->vInitialPosition.y = a4;
        pMonster->vPosition.y = a4;
        pMonster->vInitialPosition.z = a3;
        pMonster->vPosition.z = a3;
        pMonster->uSectorID = pSector;
        pMonster->uGroup = spawn->uGroup;
        pMonster->PrepareSprites(0);
        pMonster->pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
        v32 = rand();
        a3 = fixpoint_mul(stru_5C6E00->Cos(v32 % 2048), v52);
        pPosX = a3 + spawn->vPosition.x;
        a3 = fixpoint_mul(stru_5C6E00->Sin(v32 % 2048), v52);
        a4 = a3 + spawn->vPosition.y;
        a3 = spawn->vPosition.z;
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
            if (a5) pMonster->uAttributes |= ACTOR_AGGRESSOR;
            ++uNumActors;
            continue;
        }
        v37 = pIndoor->GetSector(pPosX, a4, spawn->vPosition.z);
        if (v37 == pSector) {
            v38 = BLV_GetFloorLevel(pPosX, a4, a3, v37, &uFaceID);
            v39 = v38;
            if (v38 != -30000) {
                if (abs(v38 - a3) <= 1024) {
                    a3 = v39;
                    if (a5) pMonster->uAttributes |= ACTOR_AGGRESSOR;
                    ++uNumActors;
                    continue;
                }
            }
        }
        // v53 = (char *)v53 + 1;
        // result = v53;
    }
    // while ( (signed int)v53 < NumToSpawn );
}

//----- (00438F8F) --------------------------------------------------------
void area_of_effect__damage_evaluate() {  // not damaging party correctly
    int attacker_PID_type;         // ecx@3
    signed int attacker_PID_id;             // eax@3
    unsigned int target_id;    // edi@6
    int target_type;           // eax@6
    int v10;                   // edi@8
    Vec3_int_ attacker_coord;  // ST04_12@9
    //  int v12; // ST0C_4@10
    int v15;  // edx@15
    int v19;  // edi@15
    int v23;  // edx@18
    int v24;  // eax@18
    //  int v30; // eax@29
    int v31;            // edx@29
    int v32;            // eax@29
    int v33;            // ST24_4@29
    SpriteObject *sprite_obj_ptr;  // [sp+0h] [bp-28h]@0
    int attack_index;    // [sp+10h] [bp-18h]@1
    int v44;            // [sp+14h] [bp-14h]@15
    // Vec3_int_ *pVelocity; // [sp+1Ch] [bp-Ch]@2
    signed int a1;  // [sp+20h] [bp-8h]@8
    int v48;        // [sp+24h] [bp-4h]@8

    for (attack_index = 0; attack_index < AttackerInfo.count; ++attack_index) {
        attacker_PID_type = PID_TYPE(AttackerInfo.pIDs[attack_index]);
        // pid types - enum ObjectType
        attacker_PID_id = PID_ID(AttackerInfo.pIDs[attack_index]);

        // attacker is an item (sprite)
        if (attacker_PID_type == OBJECT_Item) {
            sprite_obj_ptr = &pSpriteObjects[attacker_PID_id];
            attacker_PID_type = PID_TYPE(pSpriteObjects[attacker_PID_id].spell_caster_pid);
            attacker_PID_id = PID_ID(pSpriteObjects[attacker_PID_id].spell_caster_pid);
        }

        if (AttackerInfo.field_3EC[attack_index] & 1) {
            target_id = PID_ID(ai_near_actors_targets_pid[attacker_PID_id]);
            target_type = PID_TYPE(ai_near_actors_targets_pid[attacker_PID_id]) - 3;
            if (target_type) {
                if (target_type == 1) {  // party damage from monsters(повреждения
                                         // группе от монстров)
                    v10 = pParty->vPosition.y - AttackerInfo.pYs[attack_index];
                    a1 = pParty->vPosition.x - AttackerInfo.pXs[attack_index];
                    v48 = pParty->vPosition.y - AttackerInfo.pYs[attack_index];
                    if (a1 * a1 + v10 * v10 +
                            ((signed int)(pParty->vPosition.z +
                                          pParty->uPartyHeight) >>
                             (1 - AttackerInfo.pZs[attack_index])) *
                                ((signed int)(pParty->vPosition.z +
                                              pParty->uPartyHeight) >>
                                 (1 - AttackerInfo.pZs[attack_index])) <
                        (unsigned int)((AttackerInfo.field_324[attack_index] +
                                        32) *
                                       (AttackerInfo.field_324[attack_index] +
                                        32))) {
                        attacker_coord.x = AttackerInfo.pXs[attack_index];
                        attacker_coord.y = AttackerInfo.pYs[attack_index];
                        attacker_coord.z = AttackerInfo.pZs[attack_index];
                        if (sub_407A1C(pParty->vPosition.x, pParty->vPosition.y,
                                       pParty->vPosition.z + pParty->sEyelevel,
                                       attacker_coord))
                            DamagePlayerFromMonster(
                                AttackerInfo.pIDs[attack_index],
                                AttackerInfo.field_450[attack_index],
                                &AttackerInfo.vec_4B4[attack_index],
                                stru_50C198.which_player_to_attack(
                                    &pActors[attacker_PID_id]));
                    }
                }
            } else {  // Actor damage from monsters(повреждение местного жителя)
                if (pActors[target_id]
                        .pActorBuffs[ACTOR_BUFF_PARALYZED]
                        .Active() ||
                    pActors[target_id].CanAct()) {
                    v15 = pActors[target_id].vPosition.y -
                          AttackerInfo.pYs[attack_index];
                    a1 = pActors[target_id].vPosition.x -
                         AttackerInfo.pXs[attack_index];
                    v44 = pActors[target_id].vPosition.z;
                    v19 = AttackerInfo.field_324[attack_index] +
                          pActors[target_id].uActorRadius;
                    v48 = v15;
                    if (a1 * a1 + v15 * v15 +
                            (pActors[target_id].vPosition.z +
                             (pActors[target_id].uActorHeight >> 1) -
                             AttackerInfo.pZs[attack_index]) *
                                (pActors[target_id].vPosition.z +
                                 (pActors[target_id].uActorHeight >> 1) -
                                 AttackerInfo.pZs[attack_index]) <
                        (unsigned int)(v19 * v19)) {
                        attacker_coord.x = AttackerInfo.pXs[attack_index];
                        attacker_coord.y = AttackerInfo.pYs[attack_index];
                        attacker_coord.z = AttackerInfo.pZs[attack_index];
                        if (sub_407A1C(pActors[target_id].vPosition.x,
                                       pActors[target_id].vPosition.y,
                                       pActors[target_id].vPosition.z + 50,
                                       attacker_coord)) {
                            Vec3_int_::Normalize(&a1, &v48, &v44);
                            AttackerInfo.vec_4B4[attack_index].x = a1;
                            AttackerInfo.vec_4B4[attack_index].y = v48;
                            AttackerInfo.vec_4B4[attack_index].z = v44;
                            Actor::ActorDamageFromMonster(
                                AttackerInfo.pIDs[attack_index], target_id,
                                &AttackerInfo.vec_4B4[attack_index],
                                AttackerInfo.field_450[attack_index]);
                        }
                    }
                }
            }
        } else {  // damage from spells(повреждения от заклов(метеоритный дождь))
            v23 = pParty->vPosition.y - AttackerInfo.pYs[attack_index];
            v24 = ((signed int)pParty->uPartyHeight / 2) -
                  AttackerInfo.pZs[attack_index];
            a1 = pParty->vPosition.x - AttackerInfo.pXs[attack_index];
            v48 = pParty->vPosition.y - AttackerInfo.pYs[attack_index];
            if (a1 * a1 + v23 * v23 +
                    (pParty->vPosition.z + v24) * (pParty->vPosition.z + v24) <
                (unsigned int)((AttackerInfo.field_324[attack_index] + 32) *
                               (AttackerInfo.field_324[attack_index] +
                                32))) {  // party damage (повреждения группе)
                attacker_coord.x = AttackerInfo.pXs[attack_index];
                attacker_coord.y = AttackerInfo.pYs[attack_index];
                attacker_coord.z = AttackerInfo.pZs[attack_index];
                if (sub_407A1C(pParty->vPosition.x, pParty->vPosition.y,
                               pParty->vPosition.z + pParty->sEyelevel,
                               attacker_coord)) {
                    for (uint i = 0; i < 4; ++i) {
                        if (!pParty->pPlayers[i]
                                 .conditions_times[Condition_Dead] &&
                            !pParty->pPlayers[i]
                                 .conditions_times[Condition_Pertified] &&
                            !pParty->pPlayers[i]
                                 .conditions_times[Condition_Eradicated]) {
                            DamagePlayerFromMonster(
                                AttackerInfo.pIDs[attack_index],
                                AttackerInfo.field_450[attack_index],
                                &AttackerInfo.vec_4B4[attack_index], i);
                        }
                    }
                }
            }

            if (uNumActors >
                0) {  // actors damage(повреждения другим участникам)
                for (int actorID = 0;
                     (signed int)actorID < (signed int)uNumActors; ++actorID) {
                    if (pActors[actorID].CanAct()) {
                        // v30 = pActors[actorID].vPosition.y -
                        // AttackerInfo.pYs[attack_index];
                        a1 = pActors[actorID].vPosition.x -
                             AttackerInfo.pXs[attack_index];
                        v31 = pActors[actorID].vPosition.z;
                        v48 = pActors[actorID].vPosition.y -
                              AttackerInfo.pYs[attack_index];
                        v44 = pActors[actorID].vPosition.z;
                        v32 = (pActors[actorID].uActorHeight / 2) -
                              AttackerInfo.pZs[attack_index];
                        v33 = pActors[actorID].uActorRadius +
                              AttackerInfo.field_324[attack_index];
                        if (a1 * a1 + v48 * v48 + (v31 + v32) * (v31 + v32) <
                            (unsigned int)(v33 * v33)) {
                            attacker_coord.x = AttackerInfo.pXs[attack_index];
                            attacker_coord.y = AttackerInfo.pYs[attack_index];
                            attacker_coord.z = AttackerInfo.pZs[attack_index];
                            if (sub_407A1C(pActors[actorID].vPosition.x,
                                           pActors[actorID].vPosition.y,
                                           pActors[actorID].vPosition.z + 50,
                                           attacker_coord)) {  // что делает ф-ция?
                                Vec3_int_::Normalize(&a1, &v48, &v44);
                                AttackerInfo.vec_4B4[attack_index].x = a1;
                                AttackerInfo.vec_4B4[attack_index].y = v48;
                                AttackerInfo.vec_4B4[attack_index].z = v44;
                                switch (attacker_PID_type) {
                                    case OBJECT_Player:
                                        Actor::DamageMonsterFromParty(
                                            AttackerInfo.pIDs[attack_index],
                                            actorID,
                                            &AttackerInfo.vec_4B4[attack_index]);
                                        break;
                                    case OBJECT_Actor:
                                        if (sprite_obj_ptr &&
                                            pActors[attacker_PID_id].GetActorsRelation(
                                                &pActors[actorID]))
                                            Actor::ActorDamageFromMonster(
                                                AttackerInfo.pIDs[attack_index],
                                                actorID,
                                                &AttackerInfo
                                                     .vec_4B4[attack_index],
                                                sprite_obj_ptr->field_61);
                                        break;
                                    case OBJECT_Item:
                                        ItemDamageFromActor(
                                            AttackerInfo.pIDs[attack_index],
                                            actorID,
                                            &AttackerInfo.vec_4B4[attack_index]);
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    AttackerInfo.count = 0;
}

//----- (0043AE12) --------------------------------------------------------
double sub_43AE12(signed int a1) {
    // signed int v1; // ST00_4@1
    signed int v2;  // ecx@1
    double v3;      // st7@1
    double result;  // st7@6

    v3 = (double)a1;
    for (v2 = 0; v2 < 5; ++v2) {
        if (v3 < flt_4E4A80[v2 + 5]) break;
    }
    if (v2 <= 0 || v2 >= 5) {
        if (v2)
            result = flt_4E4A80[4];
        else
            result = flt_4E4A80[0];
    } else {
        result = (flt_4E4A80[v2] - flt_4E4A80[v2 - 1]) *
            (v3 - flt_4E4A80[v2 + 4]) /
            (flt_4E4A80[v2 + 5] - flt_4E4A80[v2 + 4]) +
            flt_4E4A80[v2];
    }
    return result;
}

//----- (0043B057) --------------------------------------------------------
void ItemDamageFromActor(unsigned int uObjID, unsigned int uActorID,
                         Vec3_int_ *pVelocity) {
    int v6;      // eax@4
    int damage;  // edi@4
    int a2a;     // [sp+Ch] [bp-4h]@8

    if (!pActors[uActorID].IsNotAlive()) {
        if (PID_TYPE(uObjID) == OBJECT_Item) {
            if (pSpriteObjects[PID_ID(uObjID)].spell_id) {
                v6 = _43AFE3_calc_spell_damage(
                    pSpriteObjects[PID_ID(uObjID)].spell_id,
                    pSpriteObjects[PID_ID(uObjID)].spell_level,
                    pSpriteObjects[PID_ID(uObjID)].spell_skill,
                    pActors[uActorID].sCurrentHP);
                damage = pActors[uActorID].CalcMagicalDamageToActor(
                    (DAMAGE_TYPE)0, v6);
                pActors[uActorID].sCurrentHP -= damage;
                if (damage) {
                    if (pActors[uActorID].sCurrentHP > 0)
                        Actor::AI_Stun(uActorID, uObjID, 0);
                    else
                        Actor::Die(uActorID);
                    a2a = 20 * damage /
                          (signed int)pActors[uActorID].pMonsterInfo.uHP;
                    if (20 * damage /
                            (signed int)pActors[uActorID].pMonsterInfo.uHP >
                        10)
                        a2a = 10;
                    if (!MonsterStats::BelongsToSupertype(
                            pActors[uActorID].pMonsterInfo.uID,
                            MONSTER_SUPERTYPE_TREANT)) {
                        pVelocity->x = fixpoint_mul(a2a, pVelocity->x);
                        pVelocity->y = fixpoint_mul(a2a, pVelocity->y);
                        pVelocity->z = fixpoint_mul(a2a, pVelocity->z);
                        pActors[uActorID].vVelocity.x =
                            50 * (short)pVelocity->x;
                        pActors[uActorID].vVelocity.y =
                            50 * (short)pVelocity->y;
                        pActors[uActorID].vVelocity.z =
                            50 * (short)pVelocity->z;
                    }
                    Actor::AddBloodsplatOnDamageOverlay(uActorID, 1, damage);
                } else {
                    Actor::AI_Stun(uActorID, uObjID, 0);
                }
            }
        }
    }
}
