#include "Engine/Objects/Actor.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/Spells.h"
#include "Engine/AttackList.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"

// should be injected into Actor but struct size cant be changed
static DecalBuilder *decal_builder = EngineIocContainer::ResolveDecalBuilder();
static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

std::vector<Actor> pActors;

stru319 stru_50C198;  // idb

std::array<uint, 5> _4DF380_hostilityRanges = {0, 1024, 2560, 5120, 10240};

std::array<int16_t, 11> word_4E8152 = {{0, 0, 0, 90, 8, 2, 70, 20, 10, 50, 30}};  // level spawn monster levels ABC

stru319::stru319() {
    this->vis = EngineIocContainer::ResolveVis();
}

//----- (0042FB5C) --------------------------------------------------------
// True if monster should play attack animation when casting this spell.
bool ShouldMonsterPlayAttackAnim(SPELL_TYPE spell_id) {
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
        default:
            return true;
    }
}

//----- (0041AF52) --------------------------------------------------------
void Actor::DrawHealthBar(Actor *actor, GUIWindow *window) {
    // bar length
    unsigned int bar_length;
    if (actor->monsterInfo.uHP <= 25)
        bar_length = 25;
    else if (actor->monsterInfo.uHP < 200)
        bar_length = actor->monsterInfo.uHP;
    else
        bar_length = 200;

    // bar colour
    GraphicsImage *bar_image = game_ui_monster_hp_green;
    if (actor->currentHP <= (0.34 * actor->monsterInfo.uHP))
        bar_image = game_ui_monster_hp_red;
    else if (actor->currentHP <= (0.67 * actor->monsterInfo.uHP))
        bar_image = game_ui_monster_hp_yellow;

    // how much of bar is filled
    unsigned int bar_filled_length = bar_length;
    if (actor->currentHP < (int)actor->monsterInfo.uHP)
        bar_filled_length = (bar_length * actor->currentHP) / actor->monsterInfo.uHP;

    // centralise for clipping and draw
    unsigned int uX = window->uFrameX + (signed int)(window->uFrameWidth - bar_length) / 2;

    render->SetUIClipRect(uX, window->uFrameY + 32, uX + bar_length, window->uFrameY + 52);
    render->DrawTextureNew(uX / 640.0f, (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_background);

    render->SetUIClipRect(uX, window->uFrameY + 32, uX + bar_filled_length,
                          window->uFrameY + 52);
    render->DrawTextureNew(uX / 640.0f, (window->uFrameY + 34) / 480.0f,
                                bar_image);

    // draw hp bar ends
    render->ResetUIClipRect();
    render->DrawTextureNew((uX - 5) / 640.0f,
                                (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_border_left);
    render->DrawTextureNew((uX + bar_length) / 640.0f,
                                (window->uFrameY + 32) / 480.0f,
                                game_ui_monster_hp_border_right);
}

void Actor::toggleFlag(signed int uActorID, ActorAttribute uFlag, bool bValue) {
    if (uActorID >= 0 && uActorID <= (signed int)(pActors.size() - 1)) {
        if (bValue) {
            pActors[uActorID].attributes |= uFlag;
        } else {
            if (uFlag == ACTOR_UNKNOW11) {
                if (pActors[uActorID].aiState == Disabled)
                    pActors[uActorID].aiState = Standing;
            }
            pActors[uActorID].attributes &= ~uFlag;
        }
    }
}

void npcSetItem(int npc, ITEM_TYPE item, int a3) {
    for (uint i = 0; i < pActors.size(); i++) {
        if (pActors[i].npcId == npc) {
            Actor::giveItem(i, item, a3);
        }
    }
}

void Actor::giveItem(signed int uActorID, ITEM_TYPE uItemID, unsigned int bGive) {
    if ((uActorID >= 0) && (signed int)uActorID <= (signed int)(pActors.size() - 1)) {
        if (bGive) {
            if (pActors[uActorID].carriedItemId == ITEM_NULL)
                pActors[uActorID].carriedItemId = uItemID;
            else if (pActors[uActorID].items[0].uItemID == ITEM_NULL)
                pActors[uActorID].items[0].uItemID = uItemID;
            else if (pActors[uActorID].items[1].uItemID == ITEM_NULL)
                pActors[uActorID].items[1].uItemID = uItemID;
        } else {
            if (pActors[uActorID].carriedItemId == uItemID)
                pActors[uActorID].carriedItemId = ITEM_NULL;
            else if (pActors[uActorID].items[0].uItemID == uItemID)
                pActors[uActorID].items[0].Reset();
            else if (pActors[uActorID].items[1].uItemID == uItemID)
                pActors[uActorID].items[1].Reset();
        }
    }
}

//----- (0040894B) --------------------------------------------------------
bool Actor::CanAct() {
    bool stoned = this->buffs[ACTOR_BUFF_STONED].Active();
    bool paralyzed = this->buffs[ACTOR_BUFF_PARALYZED].Active();
    return !(stoned || paralyzed || this->aiState == Dying ||
             this->aiState == Dead || this->aiState == Removed ||
             this->aiState == Summoned || this->aiState == Disabled);
}

//----- (004089C7) --------------------------------------------------------
bool Actor::IsNotAlive() {
    bool stoned = this->buffs[ACTOR_BUFF_STONED].Active();
    return (stoned || (aiState == Dying) || (aiState == Dead) ||
            (aiState == Removed) || (aiState == Summoned) ||
            (aiState == Disabled));
}

//----- (004086E9) --------------------------------------------------------
void Actor::SetRandomGoldIfTheresNoItem() {
    int v2;  // edi@1

    v2 = 0;
    if (this->items[3].uItemID == ITEM_NULL) {
        if (this->monsterInfo.uTreasureDiceRolls) {
            v2 = grng->randomDice(this->monsterInfo.uTreasureDiceRolls, this->monsterInfo.uTreasureDiceSides);
            if (v2) {
                this->items[3].uItemID = ITEM_GOLD_SMALL;
                this->items[3].special_enchantment =
                    (ITEM_ENCHANTMENT)v2;  // actual gold amount
            }
        }
    }
    if (grng->random(100) < this->monsterInfo.uTreasureDropChance) {
        if (this->monsterInfo.uTreasureLevel != ITEM_TREASURE_LEVEL_INVALID)
            pItemTable->generateItem(this->monsterInfo.uTreasureLevel,
                                     this->monsterInfo.uTreasureType,
                                     &this->items[2]);
    }
    this->attributes |= ACTOR_HAS_ITEM;
}

//----- (00404AC7) --------------------------------------------------------
void Actor::AI_SpellAttack(unsigned int uActorID, AIDirection *pDir,
                           SPELL_TYPE uSpellID, ABILITY_INDEX a4, CombinedSkillValue uSkillMastery) {
    GameTime spellLength = GameTime(0);

    SpriteObject sprite;
    Actor *actorPtr = &pActors[uActorID];
    int realPoints = uSkillMastery.level();
    CharacterSkillMastery masteryLevel = uSkillMastery.mastery();
    int distancemod = 3;
    int spriteId;

    static const int ONE_THIRD_PI = TrigLUT.uIntegerPi / 3;

    if (pDir->uDistance < 307.2) {
        distancemod = 0;
    } else if (pDir->uDistance < 1024) {
        distancemod = 1;
    } else if (pDir->uDistance < 2560) {
        distancemod = 2;
    }

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_FIRE_INCINERATE:
        case SPELL_AIR_LIGHTNING_BOLT:
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
            sprite.uType = SpellSpriteMapping[uSpellID];
            sprite.uObjectDescID = GetObjDescId(uSpellID);
            sprite.containing_item.Reset();
            sprite.uSpellID = uSpellID;
            sprite.spell_level = uSkillMastery.level();
            sprite.spell_skill = CHARACTER_SKILL_MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3i(0, 0, actorPtr->height / 2);
            sprite.uFacing = (short)pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.uSpriteFrameID = 0;
            sprite.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            sprite.spell_target_pid = 0;
            sprite.field_60_distance_related_prolly_lod = distancemod;
            sprite.spellCasterAbility = ABILITY_SPELL1;

            spriteId = sprite.Create(pDir->uYawAngle, pDir->uPitchAngle, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Item, spriteId));
            }
            break;

        case SPELL_FIRE_HASTE:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromHours(1).AddMinutes(realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromMinutes(40 + 2 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromMinutes(45 + 3 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HASTE].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.OrangeyRed);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_FIRE_METEOR_SHOWER:
        {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                break;
            }

            int meteors;
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                    meteors = 8;
                    break;
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    meteors = 10;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    meteors = 12;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    meteors = 14;
                    break;
                default:
                    assert(false);
                    break;
            }

            int j = 0, k = 0;
            int yaw, pitch;
            for (int i = 0; i < meteors; i++) {
                int originHeight = grng->random(1000);
                if (Vec3s(j, k, originHeight - 2500).length() <= 1.0) {
                    pitch = 0;
                    yaw = 0;
                } else {
                    pitch = TrigLUT.atan2(sqrt(j * j + k * k), originHeight - 2500);
                    yaw = TrigLUT.atan2(j, k);
                }
                sprite.containing_item.Reset();
                sprite.uType = SpellSpriteMapping[uSpellID];
                sprite.uObjectDescID = GetObjDescId(uSpellID);
                sprite.spell_level = uSkillMastery.level();
                sprite.spell_skill = CHARACTER_SKILL_MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
                sprite.vPosition = pParty->pos + Vec3i(0, 0, originHeight + 2500);
                sprite.uSpellID = SPELL_FIRE_METEOR_SHOWER;
                sprite.uAttributes = 0;
                sprite.uSectorID = 0;
                sprite.uSpriteFrameID = 0;
                sprite.spell_caster_pid = PID(OBJECT_Actor, uActorID);
                sprite.spell_target_pid = 0;
                sprite.uFacing = yaw;
                sprite.uSoundID = 0;
                sprite.field_60_distance_related_prolly_lod = distancemod;
                sprite.spellCasterAbility = ABILITY_SPELL1;

                spriteId = sprite.Create(yaw, pitch, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
                i = grng->random(1024) - 512;
                k = grng->random(1024) - 512;
            }
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Item, spriteId));
            }
            break;
        }

        case SPELL_AIR_SPARKS:
        {
            int sparks;
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                    sparks = 3;
                    break;
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    sparks = 5;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    sparks = 7;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    sparks = 9;
                    break;
                default:
                    assert(false);
                    break;
            }

            sprite.uType = SpellSpriteMapping[uSpellID];
            sprite.uObjectDescID = GetObjDescId(uSpellID);
            sprite.containing_item.Reset();
            sprite.uSpellID = SPELL_AIR_SPARKS;
            sprite.spell_level = uSkillMastery.level();
            sprite.spell_skill = CHARACTER_SKILL_MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3i(0, 0, actorPtr->height / 2);
            sprite.uFacing = pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            sprite.uSpriteFrameID = 0;
            sprite.spell_target_pid = 0;
            sprite.field_60_distance_related_prolly_lod = 3;

            int spell_spray_angle_start = ONE_THIRD_PI / -2;
            int spell_spray_angle_end = ONE_THIRD_PI / 2;
            if (spell_spray_angle_start <= spell_spray_angle_end) {
                do {
                    sprite.uFacing = spell_spray_angle_start + pDir->uYawAngle;
                    spriteId = sprite.Create(sprite.uFacing, pDir->uPitchAngle, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
                    spell_spray_angle_start += ONE_THIRD_PI / (sparks - 1);
                } while (spell_spray_angle_start <= spell_spray_angle_end);
            }
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Item, spriteId));
            }
            break;
        }

        case SPELL_AIR_SHIELD:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 5 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 15 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromHours(64 + realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_SHIELD].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            // Spell sound was missing from before
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_EARTH_STONESKIN:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 5 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 15 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromHours(64 + realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_STONESKIN].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.Cioccolato);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_SPIRIT_BLESS:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 5 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 15 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_BLESS].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_SPIRIT_FATE:
        {
            int spellPower;
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellPower = 2 * realPoints + 40;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellPower = 3 * realPoints + 60;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellPower = 6 * realPoints + 120;
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_FATE].Apply(pParty->GetPlayingTime() + GameTime::FromMinutes(5), masteryLevel, spellPower, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;
        }

        case SPELL_SPIRIT_HEROISM:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 5 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 15 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromHours(1).AddMinutes(4 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HEROISM].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_BODY_HAMMERHANDS:
            // TODO(Nik-RE-dev): calculation of duration is strange
            actorPtr->buffs[ACTOR_BUFF_HAMMERHANDS]
                .Apply(pParty->GetPlayingTime() + GameTime::FromHours(realPoints), masteryLevel, realPoints, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.JazzberryJam);
            pAudioPlayer->playSound(SOUND_51heroism03, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_BODY_POWER_CURE:
            actorPtr->currentHP += 5 * realPoints + 10;
            if (actorPtr->currentHP >= actorPtr->monsterInfo.uHP) {
                actorPtr->currentHP = actorPtr->monsterInfo.uHP;
            }
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.JazzberryJam);
            pAudioPlayer->playSound(SOUND_Fate, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_LIGHT_DISPEL_MAGIC:
            for (SpellBuff &buff : pParty->pPartyBuffs) {
                buff.Reset();
            }
            for (int i = 0; i < pParty->pCharacters.size(); i++) {
                int willCheck = pParty->pCharacters[i].GetParameterBonus(pParty->pCharacters[i].GetActualPersonality()) / 2;
                int intCheck = pParty->pCharacters[i].GetParameterBonus(pParty->pCharacters[i].GetActualIntelligence()) / 2;
                int luckCheck = pParty->pCharacters[i].GetParameterBonus(pParty->pCharacters[i].GetActualLuck()) + 30;
                if (grng->random(willCheck + intCheck + luckCheck) < 30) {
                    for (SpellBuff &buff : pParty->pCharacters[i].pCharacterBuffs) {
                        buff.Reset();
                    }
                }
            }
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_LIGHT_DAY_OF_PROTECTION:
        {
            // TODO(Nik-RE-dev): calculation of duration is strange
            int spellPower;
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromMinutes(64 + 5 * realPoints);
                    spellPower = realPoints;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromMinutes(64 + 15 * realPoints);
                    spellPower = 3 * realPoints;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromMinutes(64 + 20 * realPoints);
                    spellPower = 4 * realPoints;
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_DAY_OF_PROTECTION].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, spellPower, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.White);
            pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Actor, uActorID));
            break;
        }

        case SPELL_LIGHT_HOUR_OF_POWER:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spellLength = GameTime::FromMinutes(64 + 5 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromMinutes(64 + 15 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromMinutes(64 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HOUR_OF_POWER].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.White);
            pAudioPlayer->playSound(SOUND_9armageddon01, PID(OBJECT_Actor, uActorID));
            break;

        case SPELL_DARK_SHARPMETAL:
        {
            int pieces;
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                    pieces = 3;
                    break;
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    pieces = 5;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    pieces = 7;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    pieces = 9;
                    break;
                default:
                    assert(false);
                    break;
            }

            sprite.uType = SpellSpriteMapping[uSpellID];
            sprite.uObjectDescID = GetObjDescId(uSpellID);
            sprite.containing_item.Reset();
            sprite.uSpellID = uSpellID;
            sprite.spell_level = uSkillMastery.level();
            sprite.spell_skill = CHARACTER_SKILL_MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3i(0, 0, actorPtr->height / 2);
            sprite.uFacing = pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.spell_caster_pid = PID(OBJECT_Actor, uActorID);
            sprite.uSpriteFrameID = 0;
            sprite.spell_target_pid = 0;
            sprite.field_60_distance_related_prolly_lod = 3;

            int spell_spray_angle_start = ONE_THIRD_PI / -2;
            int spell_spray_angle_end = ONE_THIRD_PI / 2;
            if (spell_spray_angle_start <= spell_spray_angle_end) {
                do {
                    sprite.uFacing = spell_spray_angle_start + pDir->uYawAngle;
                    spriteId = sprite.Create(sprite.uFacing, pDir->uPitchAngle, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
                    spell_spray_angle_start += ONE_THIRD_PI / (pieces - 1);
                } while (spell_spray_angle_start <= spell_spray_angle_end);
            }
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, PID(OBJECT_Item, spriteId));
            }
            break;
        }

        case SPELL_DARK_PAIN_REFLECTION:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spellLength = GameTime::FromMinutes(64).AddSeconds(5 * 30 * realPoints);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spellLength = GameTime::FromMinutes(64).AddSeconds(15 * 30 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }
            actorPtr->buffs[ACTOR_BUFF_PAIN_REFLECTION].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.MediumGrey);
            pAudioPlayer->playSound(SOUND_Sacrifice2, PID(OBJECT_Actor, uActorID));
            break;

        default:
            assert(false);
            break;
    }
}

unsigned short Actor::GetObjDescId(SPELL_TYPE spellId) {
    return pObjectList->ObjectIDByItemID(SpellSpriteMapping[spellId]);  // crash here
}

bool Actor::ArePeasantsOfSameFaction(Actor *a1, Actor *a2) {
    unsigned int v2 = a1->ally;
    if (!a1->ally) v2 = (a1->monsterInfo.uID - 1) / 3 + 1;

    unsigned int v3 = a2->ally;
    if (!a2->ally) v3 = (a2->monsterInfo.uID - 1) / 3 + 1;

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
    if (a2 == 1) victim->attributes |= ACTOR_AGGRESSOR;

    for (uint i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];
        if (!actor->CanAct() || i == uActorID) continue;

        if (Actor::ArePeasantsOfSameFaction(victim, actor)) {
            v4 = abs(actor->pos.x - victim->pos.x);
            v5 = abs(actor->pos.y - victim->pos.y);
            v6 = abs(actor->pos.z - victim->pos.z);
            if (int_get_vector_length(v4, v5, v6) < 4096) {
                actor->monsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Long;
                if (a2 == 1) actor->attributes |= ACTOR_AGGRESSOR;
            }
        }
    }
}

//----- (00404874) --------------------------------------------------------
void Actor::AI_RangedAttack(unsigned int uActorID, struct AIDirection *pDir,
                            int type, ABILITY_INDEX a4) {
    char specAb;  // al@1
    int v13;      // edx@28

    SpriteObject a1;  // [sp+Ch] [bp-74h]@1

    switch (type) {
        case 1:
            a1.uType = SPRITE_ARROW_PROJECTILE;
            break;
        case 2:
            a1.uType = SPRITE_PROJECTILE_EXPLOSIVE;
            break;
        case 3:
            a1.uType = SPRITE_PROJECTILE_FIREBOLT;
            break;
        case 4:
            a1.uType = SPRITE_PROJECTILE_AIRBOLT;
            break;
        case 5:
            a1.uType = SPRITE_PROJECTILE_WATERBOLT;
            break;
        case 6:
            a1.uType = SPRITE_PROJECTILE_EARTHBOLT;
            break;
        case 7:
            a1.uType = SPRITE_PROJECTILE_530; // - one of MIND/BODY/SPIRIT
            break;
        case 8:
            a1.uType = SPRITE_PROJECTILE_525; // - one of MIND/BODY/SPIRIT
            break;
        case 9:
            a1.uType = SPRITE_PROJECTILE_520; // - one of MIND/BODY/SPIRIT
            break;
        case 10:
            a1.uType = SPRITE_PROJECTILE_LIGHTBOLT;
            break;
        case 11:
            a1.uType = SPRITE_PROJECTILE_DARKBOLT;
            break;
        case 13:
            a1.uType = SPRITE_BLASTER_PROJECTILE;
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
    a1.uSpellID = SPELL_NONE;
    a1.vPosition.x = pActors[uActorID].pos.x;
    a1.vPosition.y = pActors[uActorID].pos.y;
    a1.vPosition.z = pActors[uActorID].pos.z + (pActors[uActorID].height * 0.75);
    a1.spell_level = 0;
    a1.spell_skill = CHARACTER_SKILL_MASTERY_NONE;
    a1.uFacing = pDir->uYawAngle;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a1.vPosition);
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

    a1.spellCasterAbility = a4;
    // 1
    a1.Create(pDir->uYawAngle, pDir->uPitchAngle,
              pObjectList->pObjects[(int16_t)a1.uObjectDescID].uSpeed, 0);

    if (pActors[uActorID].monsterInfo.uSpecialAbilityType == 1) {
        specAb = pActors[uActorID].monsterInfo.uSpecialAbilityDamageDiceBonus;
        if (specAb == 2) {
            a1.vPosition.z += 40;
            v13 = pDir->uYawAngle;
        } else {
            if (specAb != 3) return;
            // 3 - for sprays of 3
            a1.Create(pDir->uYawAngle + 30, pDir->uPitchAngle,
                pObjectList->pObjects[(int16_t)a1.uObjectDescID].uSpeed, 0);
            v13 = pDir->uYawAngle - 30;
        }
        // 2 - double height stacked / spray of 3
        a1.Create(v13, pDir->uPitchAngle,
            pObjectList->pObjects[(int16_t)a1.uObjectDescID].uSpeed, 0);
    }
    return;
}

//----- (00404736) --------------------------------------------------------
void Actor::Explode(unsigned int uActorID) {  // death explosion for some actors eg gogs
    SpriteObject a1;
    a1.uType = SPRITE_OBJECT_EXPLODE;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.containing_item.Reset();
    a1.uSpellID = SPELL_NONE;
    a1.spell_level = 0;
    a1.spell_skill = CHARACTER_SKILL_MASTERY_NONE;
    a1.vPosition.x = pActors[uActorID].pos.x;
    a1.vPosition.y = pActors[uActorID].pos.y;
    a1.vPosition.z = pActors[uActorID].pos.z + (pActors[uActorID].height * 0.75);
    a1.uFacing = 0;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a1.vPosition);
    a1.uSpriteFrameID = 0;
    a1.spell_caster_pid = PID(OBJECT_Actor, uActorID);
    a1.spell_target_pid = 0;
    a1.field_60_distance_related_prolly_lod = 3;
    a1.spellCasterAbility = ABILITY_SPECIAL;
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
                             struct AIDirection *pOut, int PreferedZ) {
    signed int v4;    // eax@1
    signed int v5;    // ecx@1
    int v18;          // edx@15
    float v31;        // st7@45
    float v32;        // st6@45
    float v33;        // st7@45
    Vec3i v37;    // [sp-10h] [bp-5Ch]@15
    AIDirection v41;  // [sp+14h] [bp-38h]@46
    float outy2 = 0;      // [sp+38h] [bp-14h]@33
    float outx2 = 0;      // [sp+3Ch] [bp-10h]@33
    int outz = 0;         // [sp+40h] [bp-Ch]@6
    int outy = 0;         // [sp+44h] [bp-8h]@6
    int outx = 0;         // [sp+48h] [bp-4h]@6
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
            outx = pActors[v4].pos.x;
            outy = pActors[v4].pos.y;
            outz = pActors[v4].pos.z + (pActors[v4].height * 0.75);
            break;
        }
        case OBJECT_Character: {
            if (!v4) {
                outx = pParty->pos.x;
                outy = pParty->pos.y;
                outz =
                    pParty->pos.z + (signed int)pParty->height / 3;
                break;
            }
            if (v4 == 4) {
                v18 = pParty->_viewYaw - TrigLUT.uIntegerHalfPi;
                v37 = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                Vec3i::rotate(24, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 3) {
                v18 = pParty->_viewYaw - TrigLUT.uIntegerHalfPi;
                v37 = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                Vec3i::rotate(8, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 2) {
                v37 = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                v18 = TrigLUT.uIntegerHalfPi + pParty->_viewYaw;
                Vec3i::rotate(8, v18, 0, v37, &outx, &outy, &outz);
                break;
            }
            if (v4 == 1) {
                v37 = pParty->pos + Vec3i(0, 0, pParty->height / 3);
                v18 = TrigLUT.uIntegerHalfPi + pParty->_viewYaw;
                Vec3i::rotate(24, v18, 0, v37, &outx, &outy, &outz);
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
        case OBJECT_Face: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
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
            PreferedZ = pSpriteObjects[v5].vPosition.z;
            break;
        }
        case OBJECT_Actor: {
            outx2 = (float)pActors[v5].pos.x;
            outy2 = (float)pActors[v5].pos.y;
            PreferedZ = pActors[v5].pos.z + (pActors[v5].height * 0.75);
            break;
        }
        case OBJECT_Character: {
            outx2 = (float)pParty->pos.x;
            outy2 = (float)pParty->pos.y;
            if (!PreferedZ) PreferedZ = pParty->eyeLevel;
            PreferedZ = pParty->pos.z + PreferedZ;
            break;
        }
        case OBJECT_Decoration: {
            outx2 = (float)pLevelDecorations[v5].vPosition.x;
            outy2 = (float)pLevelDecorations[v5].vPosition.y;
            PreferedZ = pLevelDecorations[v5].vPosition.z;
            break;
        }
        default: {
            outx2 = 0.0;
            outy2 = 0.0;
            PreferedZ = 0;
            break;
        }
        case OBJECT_Face: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                outx2 = (float)((pIndoor->pFaces[v5].pBounding.x1 +
                                 pIndoor->pFaces[v5].pBounding.x2) >>
                                1);
                outy2 = (float)((pIndoor->pFaces[v5].pBounding.y1 +
                                 pIndoor->pFaces[v5].pBounding.y2) >>
                                1);
                PreferedZ = (pIndoor->pFaces[v5].pBounding.z1 +
                      pIndoor->pFaces[v5].pBounding.z2) >>
                     1;
            }
            break;
        }
    }

    v31 = (float)outx2 - (float)outx;
    v32 = (float)outy2 - (float)outy;
    a4a = (float)PreferedZ - (float)outz;
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
        pOut->uYawAngle = TrigLUT.atan2(v31, v32);
        pOut->uPitchAngle = TrigLUT.atan2(pOut->uDistanceXZ, a4a);
    }
}

//----- (00404030) --------------------------------------------------------
void Actor::AI_FaceObject(unsigned int uActorID, unsigned int uObjID, int UNUSED,
                          AIDirection *Dir_In) {
    AIDirection *Dir_Out;
    AIDirection Dir_Ret;

    if (grng->random(100) >= 5) {
        if (!Dir_In) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), uObjID, &Dir_Ret, 0);
            Dir_Out = &Dir_Ret;
        } else {
            Dir_Out = Dir_In;
        }

        pActors[uActorID].yawAngle = Dir_Out->uYawAngle;
        pActors[uActorID].currentActionTime = 0;
        pActors[uActorID].speed.z = 0;
        pActors[uActorID].speed.y = 0;
        pActors[uActorID].speed.x = 0;
        pActors[uActorID].pitchAngle = Dir_Out->uPitchAngle;
        pActors[uActorID].currentActionLength = 256;
        pActors[uActorID].aiState = Interacting;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Bored(uActorID, uObjID, Dir_In);
    }
}

//----- (00403F58) --------------------------------------------------------
void Actor::AI_StandOrBored(unsigned int uActorID, signed int uObjID,
                            int uActionLength, AIDirection *a4) {
    if (grng->random(2))  // 0 or 1
        AI_Bored(uActorID, uObjID, a4);
    else
        AI_Stand(uActorID, uObjID, uActionLength, a4);
}

//----- (00403EB6) --------------------------------------------------------
void Actor::AI_Stand(unsigned int uActorID, unsigned int object_to_face_pid,
                     unsigned int uActionLength, AIDirection *a4) {
    assert(uActorID < pActors.size());
    // Actor *actor = &pActors[uActorID];

    AIDirection a3;
    if (!a4) {
        Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), object_to_face_pid,
                                &a3, 0);
        a4 = &a3;
    }

    pActors[uActorID].aiState = Standing;
    if (!uActionLength)
        pActors[uActorID].currentActionLength = grng->random(256) + 256;  // от 256 до 256 + 256
    else
        pActors[uActorID].currentActionLength = uActionLength;
    pActors[uActorID].currentActionTime = 0;
    pActors[uActorID].yawAngle = a4->uYawAngle;
    pActors[uActorID].pitchAngle = a4->uPitchAngle;
    pActors[uActorID].speed.z = 0;
    pActors[uActorID].speed.y = 0;
    pActors[uActorID].speed.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403E61) --------------------------------------------------------
void Actor::StandAwhile(unsigned int uActorID) {
    pActors[uActorID].currentActionLength = grng->random(128) + 128;
    pActors[uActorID].currentActionTime = 0;
    pActors[uActorID].aiState = Standing;
    pActors[uActorID].speed.z = 0;
    pActors[uActorID].speed.y = 0;
    pActors[uActorID].speed.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403C6C) --------------------------------------------------------
void Actor::AI_MeleeAttack(unsigned int uActorID, signed int sTargetPid,
                           struct AIDirection *arg0) {
    int16_t v6;        // esi@6
    int16_t v7;        // edi@6
    signed int v8;     // eax@7
    Vec3i v10;     // ST04_12@9
    AIDirection *v12;  // eax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@12
    AIDirection v20;   // [sp+28h] [bp-2Ch]@12
    int v23;           // [sp+4Ch] [bp-8h]@6
    unsigned int v25;  // [sp+5Ch] [bp+8h]@13

    assert(uActorID < pActors.size());

    if (pActors[uActorID].monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY &&
        pActors[uActorID].monsterInfo.uAIType == 1) {
        Actor::AI_Stand(uActorID, sTargetPid, 0, arg0);
        return;
    }

    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v8 = PID_ID(sTargetPid);
        v6 = pActors[v8].pos.x;
        v7 = pActors[v8].pos.y;
        v23 = (int)(pActors[v8].height * 0.75 + pActors[v8].pos.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Character) {
        v6 = pParty->pos.x;
        v7 = pParty->pos.y;
        v23 = pParty->pos.z + pParty->eyeLevel;
    } else {
        Error("Should not get here");
        return;
    }

    v10.x = pActors[uActorID].pos.x;
    v10.y = pActors[uActorID].pos.y;
    v10.z = (int32_t)(pActors[uActorID].height * 0.75 +
                      pActors[uActorID].pos.z);

    if (Check_LineOfSight(Vec3i(v6, v7, v23), v10)) {
        if (arg0 != nullptr) {
            v12 = arg0;
        } else {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v12 = &a3;
        }
        pActors[uActorID].yawAngle = (short)v12->uYawAngle;
        pActors[uActorID].currentActionLength =
            pSpriteFrameTable
                ->pSpriteSFrames[pActors[uActorID].spriteIds[ANIM_AtkMelee]]
                .uAnimLength *
            8;
        pActors[uActorID].currentActionTime = 0;
        pActors[uActorID].aiState = AttackingMelee;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        v25 = pMonsterStats->pInfos[pActors[uActorID].monsterInfo.uID]
                  .uRecoveryTime;
        if (pActors[uActorID].buffs[ACTOR_BUFF_SLOWED].Active()) v25 *= 2;
        if (!pParty->bTurnBasedModeOn) {
            pActors[uActorID].monsterInfo.uRecoveryTime = (int)(debug_combat_recovery_mul * v25 * flt_debugrecmod3);
        } else {
            pActors[uActorID].monsterInfo.uRecoveryTime = v25;
        }
        pActors[uActorID].speed.z = 0;
        pActors[uActorID].speed.y = 0;
        pActors[uActorID].speed.x = 0;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, grng->random(2), 64, arg0);
    }
}

//----- (00438CF3) --------------------------------------------------------
void Actor::ApplyFineForKillingPeasant(unsigned int uActorID) {
    if (uLevelMapStatsID == 0 || !pActors[uActorID].IsPeasant()) return;

    if ((uLevelMapStatsID == 6 || uLevelMapStatsID == 7) &&
        pParty->isPartyEvil())  // celeste and bracada
        return;

    if ((uLevelMapStatsID == 5 || uLevelMapStatsID == 8) &&
        pParty->isPartyGood())  // the pit and deyja
        return;

    pParty->uFine += 100 * (pMapStats->pInfos[uLevelMapStatsID]._steal_perm +
                            pActors[uActorID].monsterInfo.uLevel +
                            pParty->GetPartyReputation());
    if (pParty->uFine < 0) pParty->uFine = 0;
    if (pParty->uFine > 4000000) pParty->uFine = 4000000;

    if (currentLocationInfo().reputation < 10000)
        currentLocationInfo().reputation++;

    if (pParty->uFine) {
        for (Character &character : pParty->pCharacters) {
            if (!character._achievedAwardsBits[Award_Fine]) {
                character._achievedAwardsBits.set(Award_Fine);
            }
        }
    }
}

//----- (0043AE80) --------------------------------------------------------
void Actor::AddOnDamageOverlay(unsigned int uActorID, int overlayType, signed int damage) {
    unsigned int actorPID = PID(OBJECT_Actor, uActorID);
    switch (overlayType) {
        case 1:
            if (damage)
                pActiveOverlayList->_4418B6(904, actorPID, 0,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 2:
            if (damage)
                pActiveOverlayList->_4418B6(905, actorPID, 0,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 3:
            if (damage)
                pActiveOverlayList->_4418B6(906, actorPID, 0,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 4:
            if (damage)
                pActiveOverlayList->_4418B6(907, actorPID, 0,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 5:
            pActiveOverlayList->_4418B6(901, actorPID, 0, actorPID, 0);
            return;
        case 6:
            pActiveOverlayList->_4418B6(902, actorPID, 0, actorPID, 0);
            return;
        case 7:
            pActiveOverlayList->_4418B6(903, actorPID, 0, actorPID, 0);
            return;
        case 8:
            pActiveOverlayList->_4418B6(900, actorPID, 0, actorPID, 0);
            return;
        case 9:
            pActiveOverlayList->_4418B6(909, actorPID, 0, actorPID, 0);
            return;
        case 10:
            pActiveOverlayList->_4418B6(908, actorPID, 0, actorPID, 0);
            return;
        default:
            return;
    }
    return;
}

//----- (0043B3E0) --------------------------------------------------------
int Actor::_43B3E0_CalcDamage(ABILITY_INDEX dmgSource) {
    int damageDiceRolls;
    int damageDiceSides;
    int damageBonus;
    SPELL_TYPE spellID;
    int spellPower = 0;
    CombinedSkillValue skill;
    int skillLevel = 0;
    CharacterSkillMastery skillMastery = CHARACTER_SKILL_MASTERY_NONE;

    switch (dmgSource) {
        case ABILITY_ATTACK1:
            if (this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
                spellPower = this->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
            if (this->buffs[ACTOR_BUFF_HEROISM].Active() && this->buffs[ACTOR_BUFF_HEROISM].power > spellPower)
                spellPower = this->buffs[ACTOR_BUFF_HEROISM].power;
            if (this->buffs[ACTOR_BUFF_HAMMERHANDS].Active())
                spellPower += this->buffs[ACTOR_BUFF_HAMMERHANDS].power;
            damageDiceRolls = this->monsterInfo.uAttack1DamageDiceRolls;
            damageDiceSides = this->monsterInfo.uAttack1DamageDiceSides;
            damageBonus = this->monsterInfo.uAttack1DamageBonus;
            break;
        case ABILITY_ATTACK2:
            damageDiceRolls = this->monsterInfo.uAttack2DamageDiceRolls;
            damageDiceSides = this->monsterInfo.uAttack2DamageDiceSides;
            damageBonus = this->monsterInfo.uAttack2DamageBonus;
            break;
        case ABILITY_SPELL1:
            spellID = this->monsterInfo.uSpell1ID;
            skill = this->monsterInfo.uSpellSkillAndMastery2;
            skillLevel = skill.level();
            skillMastery = skill.mastery();
            return CalcSpellDamage(spellID, skillLevel, skillMastery, 0);
            break;
        case ABILITY_SPELL2:
            spellID = this->monsterInfo.uSpell2ID;
            skill = this->monsterInfo.uSpellSkillAndMastery2;
            skillLevel = skill.level();
            skillMastery = skill.mastery();
            return CalcSpellDamage(spellID, skillLevel, skillMastery, 0);
            break;
        case ABILITY_SPECIAL:
            damageDiceRolls = this->monsterInfo.uSpecialAbilityDamageDiceRolls;
            damageDiceSides = this->monsterInfo.uSpecialAbilityDamageDiceSides;
            damageBonus = this->monsterInfo.uSpecialAbilityDamageDiceBonus;
        default:
            return 0;
    }

    int damage = 0;
    for (int i = 0; i < damageDiceRolls; i++)
        damage += grng->random(damageDiceSides) + 1;

    return damage + damageBonus + spellPower;
}

//----- (00438B9B) --------------------------------------------------------
bool Actor::IsPeasant() {
    unsigned int InHostile_Id;  // eax@1

    InHostile_Id = this->ally;
    if (!this->ally) InHostile_Id = (this->monsterInfo.uID - 1) / 3 + 1;
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
    Character *pPlayer;     // edi@1
    int v4;              // ebx@2
    unsigned int v5;     // eax@2
    LocationInfo *v6;  // esi@4
    int v8;              // [sp+8h] [bp-4h]@6

    pPlayer = &pParty->pCharacters[pParty->activeCharacterIndex() - 1];
    if (pPlayer->CanAct()) {
        CastSpellInfoHelpers::cancelSpellCastInProgress();
        v4 = 0;
        v5 = pMapStats->GetMapInfo(pCurrentMapName);
        if (v5) v4 = pMapStats->pInfos[v5]._steal_perm;
        v6 = &currentLocationInfo();
        pPlayer->StealFromActor(uActorID, v4, v6->reputation++);
        v8 = pPlayer->GetAttackRecoveryTime(false);
        if (v8 < engine->config->gameplay.MinRecoveryMelee.value()) v8 = engine->config->gameplay.MinRecoveryMelee.value();
        if (!pParty->bTurnBasedModeOn)
            pPlayer->SetRecoveryTime(
                (int)(debug_non_combat_recovery_mul * v8 * flt_debugrecmod3));
        pTurnEngine->ApplyPlayerAction();
    }
}

//----- (00403A60) --------------------------------------------------------
void Actor::AI_SpellAttack2(unsigned int uActorID, signed int edx0,
                            AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3i v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    int16_t v13;         // ax@10
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
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v21 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (PID_TYPE(edx0) == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v21 = pParty->pos.z + pParty->eyeLevel;
    } else {
        Error("Should not get here");
        return;
    }
    v19 = v3->height;
    v7.z = v3->pos.z + (v19 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3i(v4, v5, v21), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), a2, &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->currentActionLength = 8 * v13;
        v3->currentActionTime = 0;
        v3->aiState = AttackingRanged4;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->pInfos[v3->monsterInfo.uID].uRecoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.uRecoveryTime = pDira;
        } else {
            v3->monsterInfo.uRecoveryTime = v3->currentActionLength + (int)(debug_combat_recovery_mul * pDira * flt_debugrecmod3);
        }
        v3->speed.z = 0;
        v3->speed.y = 0;
        v3->speed.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->monsterInfo.uSpell2ID)) {
            v3->currentActionLength = 64;
            v3->currentActionTime = 0;
            v3->aiState = Fidgeting;
            v3->UpdateAnimation();
            v3->aiState = AttackingRanged4;
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
    Vec3i v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    int16_t v13;         // ax@10
    SPELL_TYPE v16;      // ecx@17
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v18;     // [sp+28h] [bp-2Ch]@9
    int v19;             // [sp+44h] [bp-10h]@6
    int v21;             // [sp+4Ch] [bp-8h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v21 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v21 = pParty->pos.z + pParty->eyeLevel;
    } else {
        Error("Should not get here");
        return;
    }
    v19 = v3->height;
    v7.z = v3->pos.z + (v19 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3i(v4, v5, v21), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->currentActionLength = 8 * v13;
        v3->currentActionTime = 0;
        v3->aiState = AttackingRanged3;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->pInfos[v3->monsterInfo.uID].uRecoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.uRecoveryTime = pDira;
        } else {
            v3->monsterInfo.uRecoveryTime = v3->currentActionLength + (int)(debug_combat_recovery_mul * pDira * flt_debugrecmod3);
        }
        v16 = v3->monsterInfo.uSpell1ID;
        v3->speed.z = 0;
        v3->speed.y = 0;
        v3->speed.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->monsterInfo.uSpell1ID)) {
            v3->currentActionLength = 64;
            v3->currentActionTime = 0;
            v3->aiState = Fidgeting;
            v3->UpdateAnimation();
            v3->aiState = AttackingRanged3;
        } else {
            v3->UpdateAnimation();
        }
    } else {
        // TODO(pskelton): Consider adding potshots if no LOS
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
    Vec3i v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    int16_t v13;         // ax@10
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v17;     // [sp+28h] [bp-2Ch]@9
    int v18;             // [sp+44h] [bp-10h]@6
    int v20;             // [sp+4Ch] [bp-8h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v20 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (PID_TYPE(sTargetPid) == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v20 = pParty->pos.z + pParty->eyeLevel;
    } else {
        Error("Should not get here");
        return;
    }
    v18 = v3->height;
    v7.z = v3->pos.z + (v18 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3i(v4, v5, v20), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->currentActionLength = 8 * v13;
        v3->currentActionTime = 0;
        v3->aiState = AttackingRanged2;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->pInfos[v3->monsterInfo.uID].uRecoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (!pParty->bTurnBasedModeOn) {
            v3->monsterInfo.uRecoveryTime = (int)(debug_combat_recovery_mul * pDira * flt_debugrecmod3);
        } else {
            v3->monsterInfo.uRecoveryTime = pDira;
        }
        v3->speed.z = 0;
        v3->speed.y = 0;
        v3->speed.x = 0;
        v3->UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64, pDir);
    }
}

//----- (00403476) --------------------------------------------------------
void Actor::AI_MissileAttack1(unsigned int uActorID, signed int sTargetPid,
                              AIDirection *pDir) {
    Actor *v3;         // ebx@1
    int xpos;            // esi@3
    int ypos;            // edi@3
    signed int v6;     // eax@4
    Vec3i v7;      // ST04_12@6
    AIDirection *v10;  // eax@9
    int16_t v14;       // ax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@10
    AIDirection v18;   // [sp+28h] [bp-2Ch]@10
    //int v19;           // [sp+44h] [bp-10h]@6
    // signed int a2; // [sp+48h] [bp-Ch]@1
    int zpos = 0;             // [sp+50h] [bp-4h]@3
    unsigned int pDira;  // [sp+5Ch] [bp+8h]@11

    v3 = &pActors[uActorID];
    // a2 = edx0;
    if (PID_TYPE(sTargetPid) == OBJECT_Actor) {
        v6 = PID_ID(sTargetPid);
        xpos = pActors[v6].pos.x;
        ypos = pActors[v6].pos.y;
        zpos = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else {
        if (PID_TYPE(sTargetPid) == OBJECT_Character) {
            xpos = pParty->pos.x;
            ypos = pParty->pos.y;
            zpos = pParty->pos.z + pParty->eyeLevel;
        } else {
            xpos = pDir->vDirection.x;
            ypos = pDir->vDirection.x;
        }
    }
    //v19 = v3->uActorHeight;
    v7.z = v3->pos.z + (v3->height * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3i(xpos, ypos, zpos), v7)
        // || Check_LineOfSight(v7.x, v7.y, v7.z, Vec3i(xpos, ypos, zpos))
        ) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v10 = &a3;
        } else {
            v10 = pDir;
        }
        v3->yawAngle = (short)v10->uYawAngle;
        v14 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .uAnimLength;
        v3->currentActionLength = 8 * v14;
        v3->currentActionTime = 0;
        v3->aiState = AttackingRanged1;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->pInfos[v3->monsterInfo.uID].uRecoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.uRecoveryTime = pDira;
        } else {
            v3->monsterInfo.uRecoveryTime = v3->currentActionLength - (int)(debug_combat_recovery_mul * pDira * -flt_debugrecmod3);
        }
        v3->speed.z = 0;
        v3->speed.y = 0;
        v3->speed.x = 0;
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
    // TODO(captainurist): yaw/pitch angles are actually initialized to 0 despite the name!
    AIDirection doNotInitializeBecauseShouldBeRandom;  // [sp+Ch] [bp-30h]@7
    int y;                                             // [sp+30h] [bp-Ch]@1
    int absx;                                          // [sp+38h] [bp-4h]@1

    x = pActors[uActor_id].initialPosition.x - pActors[uActor_id].pos.x;
    y = pActors[uActor_id].initialPosition.y - pActors[uActor_id].pos.y;
    absx = abs(x);
    absy = abs(y);
    if (absx <= absy)
        absx = absy + (absx / 2);
    else
        absx = absx + absy / 2;
    if (MonsterStats::BelongsToSupertype(pActors[uActor_id].monsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        Actor::AI_StandOrBored(uActor_id, PID(OBJECT_Character, 0), uActionLength,
                               &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    if (pActors[uActor_id].monsterInfo.uMovementType ==
        MONSTER_MOVEMENT_TYPE_GLOBAL &&
        absx < 128) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256,
                        &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    absx += (grng->random(0x10) * radius) / 16;
    v9 = (TrigLUT.uIntegerDoublePi - 1) & TrigLUT.atan2(x, y);
    if (grng->random(100) < 25) {
        Actor::StandAwhile(uActor_id);
        return;
    }
    v10 = v9 + grng->random(256) - 128;
    if (abs(v10 - pActors[uActor_id].yawAngle) > 256 &&
        !(pActors[uActor_id].attributes & ACTOR_ANIMATION)) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256,
                        &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    pActors[uActor_id].yawAngle = v10;
    if (pActors[uActor_id].moveSpeed)
        pActors[uActor_id].currentActionLength =
            32 * absx / pActors[uActor_id].moveSpeed;
    else
        pActors[uActor_id].currentActionLength = 0;
    pActors[uActor_id].currentActionTime = 0;
    pActors[uActor_id].aiState = Tethered;
    if (vrng->random(100) < 2) {
        Actor::playSound(uActor_id, ACTOR_BORED_SOUND);
    }
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
    int16_t v10; // cx@15
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
        v4->pMonsterInfo.uMovementType = MONSTER_MOVEMENT_TYPE_STATIONARY;
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
    int16_t v7;      // ax@16
    AIDirection a3;  // [sp+Ch] [bp-40h]@16

    if (pActors[uActorID].aiState == Fleeing)
        pActors[uActorID].attributes |= ACTOR_FLEEING;
    if (pActors[uActorID].monsterInfo.uHostilityType != 4) {
        pActors[uActorID].attributes &= ~ACTOR_UNKNOWN_4;
        pActors[uActorID].monsterInfo.uHostilityType = MonsterInfo::Hostility_Long;
    }
    if (pActors[uActorID].buffs[ACTOR_BUFF_CHARM].Active())
        pActors[uActorID].buffs[ACTOR_BUFF_CHARM].Reset();
    if (pActors[uActorID].buffs[ACTOR_BUFF_AFRAID].Active())
        pActors[uActorID].buffs[ACTOR_BUFF_AFRAID].Reset();
    if (stunRegardlessOfState ||
        (pActors[uActorID].aiState != Stunned &&
         pActors[uActorID].aiState != AttackingRanged1 &&
         pActors[uActorID].aiState != AttackingRanged2 &&
         pActors[uActorID].aiState != AttackingRanged3 &&
         pActors[uActorID].aiState != AttackingRanged4 &&
         pActors[uActorID].aiState != AttackingMelee)) {
        Actor::GetDirectionInfo(PID(OBJECT_Actor, uActorID), edx0, &a3, 0);
        // v10 = &a3;
        pActors[uActorID].yawAngle = (short)a3.uYawAngle;
        v7 = pSpriteFrameTable
                 ->pSpriteSFrames[pActors[uActorID].spriteIds[ANIM_GotHit]]
                 .uAnimLength;
        pActors[uActorID].currentActionTime = 0;
        pActors[uActorID].aiState = Stunned;
        pActors[uActorID].currentActionLength = 8 * v7;
        Actor::playSound(uActorID, ACTOR_STUNNED_SOUND);
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

    actor->currentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[actor->spriteIds[ANIM_Bored]]
                .uAnimLength;

    v7 = TrigLUT.atan2(actor->pos.x - pCamera3D->vCameraPos.x, actor->pos.y - pCamera3D->vCameraPos.y);
    v9 = TrigLUT.uIntegerPi + actor->yawAngle +
         ((signed int)TrigLUT.uIntegerPi >> 3) - v7;

    if (v9 & 0x700) {  // turned away - just stand
        Actor::AI_Stand(uActorID, uObjID, actor->currentActionLength, a4);
    } else {  // facing player - play bored anim
        actor->aiState = Fidgeting;
        actor->currentActionTime = 0;
        actor->yawAngle = a4->uYawAngle;
        actor->speed.z = 0;
        actor->speed.y = 0;
        actor->speed.x = 0;
        if (vrng->random(100) < 5) {
            Actor::playSound(uActorID, ACTOR_BORED_SOUND);
        }
        actor->UpdateAnimation();
    }
}

//----- (00402F27) --------------------------------------------------------
void Actor::resurrect(unsigned int uActorID) {
    assert(uActorID < pActors.size());
    Actor *pActor = &pActors[uActorID];
    pActor->currentActionTime = 0;
    pActor->aiState = Resurrected;
    pActor->currentActionAnimation = ANIM_Dying;
    pActor->currentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[pActor->spriteIds[ANIM_Dying]]
                .uAnimLength;
    pActor->currentHP = (short)pActor->monsterInfo.uHP;
    Actor::playSound(uActorID, ACTOR_DEATH_SOUND);
    pActor->UpdateAnimation();

    pActor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
    // TODO(pskelton): vanilla behaviour but does it make sense to drop all carried treasure
    pActor->monsterInfo.uTreasureDropChance = 0;
    pActor->monsterInfo.uTreasureDiceRolls = 0;
    pActor->monsterInfo.uTreasureDiceSides = 0;
    pActor->monsterInfo.uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    pActor->monsterInfo.uTreasureType = 0;
    pActor->ally = 9999;
    pActor->ResetAggressor();  // ~0x80000
    pActor->group = 0;
    pActor->buffs[ACTOR_BUFF_BERSERK].Reset();
    pActor->buffs[ACTOR_BUFF_CHARM].Reset();
    pActor->buffs[ACTOR_BUFF_ENSLAVED].Reset();
}

//----- (00402D6E) --------------------------------------------------------
void Actor::Die(unsigned int uActorID) {
    Actor *actor = &pActors[uActorID];

    actor->currentActionTime = 0;
    actor->aiState = Dying;
    actor->currentActionAnimation = ANIM_Dying;
    actor->currentHP = 0;
    actor->currentActionLength =
        8 * pSpriteFrameTable->pSpriteSFrames[actor->spriteIds[ANIM_Dying]]
                .uAnimLength;
    actor->buffs[ACTOR_BUFF_PARALYZED].Reset();
    actor->buffs[ACTOR_BUFF_STONED].Reset();
    Actor::playSound(uActorID, ACTOR_DEATH_SOUND);
    actor->UpdateAnimation();

    for (HOUSE_ID house : allTownhallHouses())
        if (pParty->monster_id_for_hunting[house] == actor->monsterInfo.uID)
            pParty->monster_for_hunting_killed[house] = true;

    for (SpellBuff &buff : actor->buffs)
        buff.Reset();

    ItemGen drop;
    drop.Reset();
    switch (actor->monsterInfo.uID) {
        case MONSTER_HARPY_1:
        case MONSTER_HARPY_2:
        case MONSTER_HARPY_3:
            drop.uItemID = ITEM_REAGENT_HARPY_FEATHER;
            break;

        case MONSTER_OOZE_1:
        case MONSTER_OOZE_2:
        case MONSTER_OOZE_3:
            drop.uItemID = ITEM_REAGENT_VIAL_OF_OOZE_ENDOPLASM;
            break;

        case MONSTER_TROLL_1:
        case MONSTER_TROLL_2:
        case MONSTER_TROLL_3:
            drop.uItemID = ITEM_REAGENT_VIAL_OF_TROLL_BLOOD;
            break;

        case MONSTER_DEVIL_1:
        case MONSTER_DEVIL_2:
        case MONSTER_DEVIL_3:
            drop.uItemID = ITEM_REAGENT_VIAL_OF_DEVIL_ICHOR;
            break;

        case MONSTER_DRAGON_1:
        case MONSTER_DRAGON_2:
        case MONSTER_DRAGON_3:
            drop.uItemID = ITEM_REAGENT_DRAGONS_EYE;
            break;
    }

    if (grng->random(100) < 20 && drop.uItemID != ITEM_NULL) {
        SpriteObject::dropItemAt((SPRITE_OBJECT_TYPE)pItemTable->pItems[drop.uItemID].uSpriteID,
                                 actor->pos + Vec3i(0, 0, 16), grng->random(200) + 200, 1, true, 0, &drop);
    }

    if (actor->monsterInfo.uSpecialAbilityType == MONSTER_SPECIAL_ABILITY_EXPLODE) {
        Actor::Explode(uActorID);
    }
}

void Actor::playSound(unsigned int uActorID, ActorSounds uSoundID) {
    SoundID sound_sample_id = (SoundID)pActors[uActorID].soundSampleIds[std::to_underlying(uSoundID)];
    if (sound_sample_id) {
        if (!pActors[uActorID].buffs[ACTOR_BUFF_SHRINK].Active()) {
            pAudioPlayer->playSound(sound_sample_id, PID(OBJECT_Actor, uActorID));
        } else {
            // TODO(pskelton): looks incomplete - sounds meant to change depending on actor size
            switch (pActors[uActorID].buffs[ACTOR_BUFF_SHRINK].power) {
                case 1:
                    pAudioPlayer->playSound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0);
                    break;
                case 2:
                    pAudioPlayer->playSound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0);
                    break;
                case 3:
                case 4:
                    pAudioPlayer->playSound(sound_sample_id, PID(OBJECT_Actor, uActorID), 0, 0, 0);
                    break;
                default:
                    pAudioPlayer->playSound(sound_sample_id, PID(OBJECT_Actor, uActorID));
                    break;
            }
        }
    }
}

//----- (00402AD7) --------------------------------------------------------
void Actor::AI_Pursue1(unsigned int uActorID, unsigned int a2, signed int arg0,
                       signed int uActionLength, AIDirection *pDir) {
    Actor *v7;         // ebx@1
    unsigned int v8;   // ecx@1
    AIDirection *v10;  // esi@6
    AIDirection a3;    // [sp+Ch] [bp-5Ch]@7
    unsigned int v18;  // [sp+64h] [bp-4h]@1

    int WantedZ = 0;
    v7 = &pActors[uActorID];
    v8 = PID(OBJECT_Actor, uActorID);
    if (v7->monsterInfo.uFlying != 0 && !pParty->bFlying) {
        if (v7->monsterInfo.uMissleAttack1Type)
            WantedZ = v7->radius + 512; // hovering above ground for missle
        else
            WantedZ = pParty->height; // eye height for melee
    }

    if (pDir == nullptr) {
        Actor::GetDirectionInfo(v8, a2, &a3, WantedZ);
        v10 = &a3;
    } else {
        v10 = pDir;
    }
    if (MonsterStats::BelongsToSupertype(v7->monsterInfo.uID,
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
    if (v7->moveSpeed == 0) {
        Actor::AI_Stand(uActorID, a2, uActionLength, v10);
        return;
    }
    if (arg0 % 2)
        v18 = -16;
    else
        v18 = 16;

    v7->yawAngle = TrigLUT.atan2(pParty->pos.x + TrigLUT.cos(v18 + TrigLUT.uIntegerPi + v10->uYawAngle) * v10->uDistanceXZ - v7->pos.x,
                                  pParty->pos.y + TrigLUT.sin(v18 + TrigLUT.uIntegerPi + v10->uYawAngle) * v10->uDistanceXZ - v7->pos.y);
    if (uActionLength)
        v7->currentActionLength = uActionLength;
    else
        v7->currentActionLength = 128;
    v7->pitchAngle = (short)v10->uPitchAngle;
    v7->aiState = Pursuing;
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
                                    v5->monsterInfo.uFlying);
            a4 = &a3;
        }
        Actor::GetDirectionInfo(v7, 4u, &v10, 0);
        v13 = &v10;
        if (MonsterStats::BelongsToSupertype(v5->monsterInfo.uID,
                                             MONSTER_SUPERTYPE_TREANT) ||
            PID_TYPE(sTargetPid) == OBJECT_Actor && v13->uDistance < 307.2) {
            if (!uActionLength) uActionLength = 256;
            Actor::AI_StandOrBored(uActorID, 4, uActionLength, v13);
        } else {
            if (v5->moveSpeed)
                v5->currentActionLength =
                    (signed int)(a4->uDistanceXZ << 7) / v5->moveSpeed;
            else
                v5->currentActionLength = 0;
            if (v5->currentActionLength > 256) v5->currentActionLength = 256;
            v5->yawAngle =
                (short)TrigLUT.uIntegerHalfPi + (short)a4->uYawAngle;
            v5->yawAngle =
                (short)TrigLUT.uDoublePiMask &
                (v5->yawAngle + grng->random(TrigLUT.uIntegerPi));
            v5->currentActionTime = 0;
            v5->pitchAngle = (short)a4->uPitchAngle;
            v5->aiState = Fleeing;
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
    int16_t v13;    // cx@19
    uint16_t v14;  // ax@25
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection v18;       // [sp+28h] [bp-24h]@8

    v6 = 0;
    v7 = &pActors[uActorID];
    v8 = PID(OBJECT_Actor, uActorID);
    if (v7->monsterInfo.uFlying != 0 && !pParty->bFlying) {
        if (v7->monsterInfo.uMissleAttack1Type &&
            uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
            v6 = v7->radius + 512;
        else
            v6 = pParty->height;
    }
    v10 = pDir;
    if (!pDir) {
        Actor::GetDirectionInfo(v8, a2, &a3, v6);
        v10 = &a3;
    }
    if (MonsterStats::BelongsToSupertype(v7->monsterInfo.uID,
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
        v7->currentActionLength = uActionLength;
    } else {
        v13 = v7->moveSpeed;
        if (v13)
            v7->currentActionLength =
                (signed int)(v10->uDistanceXZ << 7) / v13;
        else
            v7->currentActionLength = 0;
        if (v7->currentActionLength > 32) v7->currentActionLength = 32;
    }
    v7->yawAngle = (short)v10->uYawAngle;
    v14 = (short)v10->uPitchAngle;
    v7->currentActionTime = 0;
    v7->pitchAngle = v14;
    v7->aiState = Pursuing;
    v7->UpdateAnimation();
}

//----- (00402686) --------------------------------------------------------
void Actor::AI_Pursue3(unsigned int uActorID, unsigned int a2,
                       signed int uActionLength, AIDirection *a4) {
    int v5;                // eax@1
    Actor *v6;             // ebx@1
    int v7;                // ecx@1
    int16_t v12;    // cx@19
    int16_t v14;           // ax@25
    uint16_t v16;  // ax@28
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection *v20;      // [sp+28h] [bp-24h]@8

    v5 = 0;
    v6 = &pActors[uActorID];
    v7 = PID(OBJECT_Actor, uActorID);
    if (v6->monsterInfo.uFlying != 0 && !pParty->bFlying) {
        if (v6->monsterInfo.uMissleAttack1Type &&
            uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
            v5 = v6->radius + 512;
        else
            v5 = pParty->height;
    }
    if (!a4) {
        Actor::GetDirectionInfo(v7, a2, &a3, v5);
        v20 = &a3;
    }
    if (MonsterStats::BelongsToSupertype(v6->monsterInfo.uID,
                                         MONSTER_SUPERTYPE_TREANT)) {
        if (!uActionLength) uActionLength = 256;
        return Actor::AI_StandOrBored(uActorID, 4, uActionLength, a4);
    }
    if (a4->uDistance < 307.2) {
        if (!uActionLength) uActionLength = 256;
        return Actor::AI_StandOrBored(uActorID, a2, uActionLength, a4);
    }
    if (uActionLength) {
        v6->currentActionLength = uActionLength + grng->random(uActionLength);
    } else {
        v12 = v6->moveSpeed;
        if (v12)
            v6->currentActionLength = (signed int)(a4->uDistanceXZ << 7) / v12;
        else
            v6->currentActionLength = 0;
        if (v6->currentActionLength > 128) v6->currentActionLength = 128;
    }
    v14 = (short)a4->uYawAngle;
    if (grng->random(2))
        v14 += 256;
    else
        v14 -= 256;
    v6->yawAngle = v14;
    v16 = (short)a4->uPitchAngle;
    v6->currentActionTime = 0;
    v6->pitchAngle = v16;
    v6->aiState = Pursuing;
    if (vrng->random(100) < 2) {
        Actor::playSound(uActorID, ACTOR_STUNNED_SOUND);
    }
    v6->UpdateAnimation();
}

//----- (00401221) --------------------------------------------------------
void Actor::_SelectTarget(unsigned int uActorID, int *OutTargetPID,
                          bool can_target_party) {
    int v5;                     // ecx@1
    signed int v10;             // eax@13
    uint v11;                   // ebx@16
    uint v12;                   // eax@16
    signed int v14;             // eax@31
    uint v15;                   // edi@43
    //uint v16;                   // ebx@45
    //uint v17;                   // eax@45
    signed int closestId;       // [sp+14h] [bp-1Ch]@1
    uint v23;                   // [sp+1Ch] [bp-14h]@16
    unsigned int lowestRadius;  // [sp+24h] [bp-Ch]@1
    uint v27;                   // [sp+2Ch] [bp-4h]@16
    //uint v28;                   // [sp+2Ch] [bp-4h]@45

    lowestRadius = UINT_MAX;
    v5 = 0;
    // TODO(pskelton): change to PID_INVALID and sort logic in calling funcs
    *OutTargetPID = 0;
    closestId = 0;
    assert(uActorID < pActors.size());
    Actor *thisActor = &pActors[uActorID];

    for (uint i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];
        if (actor->aiState == Dead || actor->aiState == Dying ||
            actor->aiState == Removed || actor->aiState == Summoned ||
            actor->aiState == Disabled || uActorID == i)
            continue;

        if (thisActor->lastCharacterIdToHit == 0 ||
            PID(OBJECT_Actor, v5) != thisActor->lastCharacterIdToHit) {
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == 0) continue;
        } else if (thisActor->IsNotAlive()) {
            thisActor->lastCharacterIdToHit = 0;
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == 0) continue;
        } else {
            if ((actor->group != 0 || thisActor->group != 0) &&
                actor->group == thisActor->group)
                continue;
            v10 = 4;
        }
        if (thisActor->monsterInfo.uHostilityType)
            v10 = pMonsterStats->pInfos[thisActor->monsterInfo.uID]
                      .uHostilityType;
        v11 = _4DF380_hostilityRanges[v10];
        v23 = abs(thisActor->pos.x - actor->pos.x);
        v27 = abs(thisActor->pos.y - actor->pos.y);
        v12 = abs(thisActor->pos.z - actor->pos.z);
        if (v23 <= v11 && v27 <= v11 && v12 <= v11 &&
            Detect_Between_Objects(PID(OBJECT_Actor, i),
                                            PID(OBJECT_Actor, uActorID)) &&
            v23 * v23 + v27 * v27 + v12 * v12 < lowestRadius) {
            lowestRadius = v23 * v23 + v27 * v27 + v12 * v12;
            closestId = i;
        }
    }

    if (lowestRadius != UINT_MAX) {
        *OutTargetPID = PID(OBJECT_Actor, closestId);
    }

    if (can_target_party && !pParty->Invisible()) {
        if (thisActor->ActorEnemy() &&
            !thisActor->buffs[ACTOR_BUFF_ENSLAVED].Active() &&
            !thisActor->buffs[ACTOR_BUFF_CHARM].Active() &&
            !thisActor->buffs[ACTOR_BUFF_SUMMONED].Active())
            v14 = 4;
        else
            v14 = thisActor->GetActorsRelation(0);
        if (v14 != 0) {
            if (!thisActor->monsterInfo.uHostilityType)
                v15 = _4DF380_hostilityRanges[v14];
            else
                v15 = _4DF380_hostilityRanges[4];
            uint v16 = abs(thisActor->pos.x - pParty->pos.x);
            uint v28 = abs(thisActor->pos.y - pParty->pos.y);
            uint v17 = abs(thisActor->pos.z - pParty->pos.z);
            if (v16 <= v15 && v28 <= v15 && v17 <= v15 &&
                (v16 * v16 + v28 * v28 + v17 * v17 < lowestRadius)) {
                *OutTargetPID = PID(OBJECT_Character, 0);
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
        if (otherActPtr->group != 0 && this->group != 0 &&
            otherActPtr->group == this->group)
            return 0;
    }

    if (this->buffs[ACTOR_BUFF_BERSERK].Active()) return 4;
    thisAlly = this->ally;
    if (this->buffs[ACTOR_BUFF_ENSLAVED].Active() || thisAlly == 9999)
        thisGroup = 0;
    else if (thisAlly > 0)
        thisGroup = thisAlly;
    else
        thisGroup = (this->monsterInfo.uID - 1) / 3 + 1;

    if (otherActPtr) {
        if (otherActPtr->buffs[ACTOR_BUFF_BERSERK].Active()) return 4;
        otherAlly = otherActPtr->ally;
        if (otherActPtr->buffs[ACTOR_BUFF_ENSLAVED].Active() ||
            otherAlly == 9999)
            otherGroup = 0;
        else if (otherAlly > 0)
            otherGroup = otherAlly;
        else
            otherGroup = (otherActPtr->monsterInfo.uID - 1) / 3 + 1;
    } else {
        otherGroup = 0;
    }

    if (this->buffs[ACTOR_BUFF_CHARM].Active() && !otherGroup ||
        otherActPtr && otherActPtr->buffs[ACTOR_BUFF_CHARM].Active() &&
        !thisGroup)
        return 0;
    if (!this->buffs[ACTOR_BUFF_ENSLAVED].Active() &&
        this->ActorEnemy() && !otherGroup)
        return 4;
    if (thisGroup >= 89 || otherGroup >= 89) return 0;

    if (thisGroup == 0) {
        if ((!otherActPtr || this->buffs[ACTOR_BUFF_ENSLAVED].Active() &&
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
    switch (aiState) {
        case Tethered:
            currentActionAnimation = ANIM_Walking;
            break;

        case AttackingMelee:
            currentActionAnimation = ANIM_AtkMelee;
            attributes |= ACTOR_ANIMATION;
            break;

        case AttackingRanged1:
        case AttackingRanged2:
        case AttackingRanged3:
        case AttackingRanged4:
            currentActionAnimation = ANIM_AtkRanged;
            attributes |= ACTOR_ANIMATION;
            break;

        case Dying:
        case Resurrected:
            currentActionAnimation = ANIM_Dying;
            attributes |= ACTOR_ANIMATION;
            break;

        case Pursuing:
        case Fleeing:
            currentActionAnimation = ANIM_Walking;
            attributes |= ACTOR_ANIMATION;
            break;

        case Stunned:
            currentActionAnimation = ANIM_GotHit;
            attributes |= ACTOR_ANIMATION;
            break;

        case Fidgeting:
            currentActionAnimation = ANIM_Bored;
            attributes |= ACTOR_ANIMATION;
            break;

        case Standing:
        case Interacting:
        case Summoned:
            currentActionAnimation = ANIM_Standing;
            attributes |= ACTOR_ANIMATION;
            break;

        case Dead:
            if (pSpriteFrameTable->pSpriteSFrames[spriteIds[ANIM_Dead]].hw_sprites[0] == nullptr)
                aiState = Removed;
            else
                currentActionAnimation = ANIM_Dead;
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
    int id = this->id;
    *this = Actor();
    this->id = id;
}

//----- (0045959A) --------------------------------------------------------
void Actor::PrepareSprites(char load_sounds_if_bit1_set) {
    MonsterDesc *v3;  // esi@1
    MonsterInfo *v9;  // [sp+84h] [bp-10h]@1

    v3 = &pMonsterList->pMonsters[monsterInfo.uID - 1];
    v9 = &pMonsterStats->pInfos[monsterInfo.uID /*- 1 + 1*/];
    // v12 = pSpriteIDs;
    // Source = (char *)v3->pSpriteNames;
    // do
    for (ActorAnimation i : spriteIds.indices()) {
        // strcpy(pSpriteName, v3->pSpriteNames[i]);
        spriteIds[i] = pSpriteFrameTable->FastFindSprite(v3->pSpriteNames[i]);
        pSpriteFrameTable->InitializeSprite(spriteIds[i]);
    }
    height = v3->uMonsterHeight;
    radius = v3->uMonsterRadius;
    moveSpeed = v9->uBaseSpeed;
    if (!(load_sounds_if_bit1_set & 1)) {
        for (int i = 0; i < 4; ++i) soundSampleIds[i] = v3->pSoundSampleIDs[i];
    }
}

//----- (00459667) --------------------------------------------------------
void Actor::Remove() { this->aiState = Removed; }

//----- (0043B1B0) --------------------------------------------------------
void Actor::ActorDamageFromMonster(signed int attacker_id,
                                   unsigned int actor_id, Vec3i *pVelocity,
                                   ABILITY_INDEX a4) {
    int v4;            // ebx@1
    int dmgToRecv;     // qax@8
    int v12;    // ecx@20
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
            pActors[actor_id].lastCharacterIdToHit = attacker_id;
            if (pActors[actor_id].aiState == Fleeing)
                pActors[actor_id].attributes |= ACTOR_FLEEING;
            if (pActors[PID_ID(attacker_id)]._4273BB_DoesHitOtherActor(
                    &pActors[actor_id], v4, 0)) {
                dmgToRecv = pActors[PID_ID(attacker_id)]._43B3E0_CalcDamage(a4);
                if (pActors[PID_ID(attacker_id)]
                        .buffs[ACTOR_BUFF_SHRINK]
                        .Active()) {
                    if (pActors[PID_ID(attacker_id)]
                            .buffs[ACTOR_BUFF_SHRINK]
                            .power > 0)
                        dmgToRecv =
                            dmgToRecv / pActors[PID_ID(attacker_id)]
                                            .buffs[ACTOR_BUFF_SHRINK]
                                            .power;
                }
                if (pActors[actor_id].buffs[ACTOR_BUFF_STONED].Active())
                    dmgToRecv = 0;
                if (a4 == ABILITY_ATTACK1) {
                    v12 =
                        pActors[PID_ID(attacker_id)].monsterInfo.uAttack1Type;
                } else if (a4 == ABILITY_ATTACK2) {
                    v12 =
                        pActors[PID_ID(attacker_id)].monsterInfo.uAttack2Type;
                    if (pActors[actor_id]
                            .buffs[ACTOR_BUFF_SHIELD]
                            .Active())
                        dmgToRecv = dmgToRecv / 2;
                } else if (a4 == ABILITY_SPELL1) {
                    v12 = pSpellStats
                        ->pInfos[pActors[PID_ID(attacker_id)].monsterInfo.uSpell1ID]
                        .uSchool;
                } else if (a4 == ABILITY_SPELL2) {
                    v12 = pSpellStats
                        ->pInfos[pActors[PID_ID(attacker_id)].monsterInfo.uSpell2ID]
                        .uSchool;
                } else if (a4 == ABILITY_SPECIAL) {
                    v12 = pActors[PID_ID(attacker_id)]
                        .monsterInfo.field_3C_some_special_attack;
                } else {
                    v12 = 4;
                }
                finalDmg = pActors[actor_id].CalcMagicalDamageToActor(
                    (DAMAGE_TYPE)v12, dmgToRecv);
                pActors[actor_id].currentHP -= finalDmg;
                if (finalDmg) {
                    if (pActors[actor_id].currentHP > 0)
                        Actor::AI_Stun(actor_id, attacker_id, 0);
                    else
                        Actor::Die(actor_id);
                    Actor::AggroSurroundingPeasants(actor_id, 0);
                    pushDistance =
                        20 * finalDmg / pActors[actor_id].monsterInfo.uHP;
                    if (pushDistance > 10) pushDistance = 10;
                    if (!MonsterStats::BelongsToSupertype(
                            pActors[actor_id].monsterInfo.uID,
                            MONSTER_SUPERTYPE_TREANT)) {
                        pVelocity->x =
                            (int32_t)fixpoint_mul(pushDistance, pVelocity->x);
                        pVelocity->y =
                            (int32_t)fixpoint_mul(pushDistance, pVelocity->y);
                        pVelocity->z =
                            (int32_t)fixpoint_mul(pushDistance, pVelocity->z);
                        pActors[actor_id].speed.x =
                            50 * (short)pVelocity->x;
                        pActors[actor_id].speed.y =
                            50 * (short)pVelocity->y;
                        pActors[actor_id].speed.z =
                            50 * (short)pVelocity->z;
                    }
                    Actor::AddOnDamageOverlay(actor_id, 1, finalDmg);
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
    uint8_t extraSummonLevel;  // al@1
    int summonMonsterBaseType;         // esi@1
    int v5;                            // edx@2
    int v7;                            // edi@10
    MonsterInfo *v9;                   // ebx@10
    // MonsterDesc *v10; // edi@10
    int v13;                 // ebx@10
    int64_t v15;                 // edi@10
    int64_t v17;                 // ebx@10
    unsigned int v19;        // qax@10
    unsigned int monsterId;  // [sp+10h] [bp-18h]@8
    int v27;                 // [sp+18h] [bp-10h]@10
    int actorSector;         // [sp+1Ch] [bp-Ch]@8

    actorSector = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        actorSector = pIndoor->GetSector(this->pos);

    v19 = this->ally;
    if (!this->ally) {
        monsterId = this->monsterInfo.uID - 1;
        v19 = (uint)(monsterId * 0.33333334);
    }
    v27 = uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ? 128 : 64;
    v13 = grng->random(2048);
    v15 = TrigLUT.cos(v13) * v27 + this->pos.x;
    v17 = TrigLUT.sin(v13) * v27 + this->pos.y;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        int sectorId = pIndoor->GetSector(v15, v17, this->pos.z);
        if (sectorId != actorSector) return;
        int z = BLV_GetFloorLevel(Vec3i(v15, v17, v27), sectorId);
        if (z != -30000) return;
        if (abs(z - v27) > 1024) return;
    }

    extraSummonLevel = this->monsterInfo.uSpecialAbilityDamageDiceRolls;
    summonMonsterBaseType = this->monsterInfo.field_3C_some_special_attack;
    if (extraSummonLevel) {
        if (extraSummonLevel >= 1 && extraSummonLevel <= 3)
            summonMonsterBaseType =
                summonMonsterBaseType + extraSummonLevel - 1;
    } else {
        v5 = grng->random(100);
        if (v5 >= 90)
            summonMonsterBaseType += 2;
        else if (v5 >= 60)
            summonMonsterBaseType += 1;
    }
    v7 = summonMonsterBaseType - 1;
    Actor *actor = AllocateActor(true);
    if (!actor)
        return;

    v9 = &pMonsterStats->pInfos[v7 + 1];
    actor->name = v9->pName;
    actor->currentHP = v9->uHP;
    actor->monsterInfo = *v9;
    actor->word_000086_some_monster_id = summonMonsterBaseType;
    actor->radius = pMonsterList->pMonsters[v7].uMonsterRadius;
    actor->height = pMonsterList->pMonsters[v7].uMonsterHeight;
    actor->monsterInfo.uTreasureDiceRolls = 0;
    actor->monsterInfo.uTreasureType = 0;
    actor->monsterInfo.uExp = 0;
    actor->moveSpeed = pMonsterList->pMonsters[v7].uMovementSpeed;

    actor->initialPosition.x = v15;
    actor->initialPosition.y = v17;
    actor->initialPosition.z = this->pos.z;
    actor->pos.x = v15;
    actor->pos.y = v17;
    actor->pos.z = this->pos.z;

    actor->tetherDistance = 256;
    actor->sectorId = actorSector;
    actor->PrepareSprites(0);
    actor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
    actor->ally = v19;
    actor->currentActionTime = 0;
    actor->group = this->group;
    actor->aiState = Summoned;
    actor->currentActionLength = 256;
    actor->UpdateAnimation();

    ++this->monsterInfo.uSpecialAbilityDamageDiceBonus;
    if (ActorEnemy())
        actor->attributes |= ACTOR_AGGRESSOR;
    actor->summonerId = PID(OBJECT_Actor, summonerId);
}

//----- (00401A91) --------------------------------------------------------
void Actor::UpdateActorAI() {
    double v42;              // st7@176
    double v43;              // st6@176
    ABILITY_INDEX v45;                 // eax@192
    uint8_t v46;     // cl@197
    signed int v47;          // st7@206
    uint v58;                // st7@246
    unsigned int v65{};        // [sp-10h] [bp-C0h]@144
    int v70;                 // [sp-10h] [bp-C0h]@213
    AIDirection v72;         // [sp+0h] [bp-B0h]@246
    ObjectType target_pid_type;     // [sp+70h] [bp-40h]@83
    AIDirection *pDir;       // [sp+7Ch] [bp-34h]@129
    int v81;                 // [sp+9Ch] [bp-14h]@100
    signed int target_pid;   // [sp+ACh] [bp-4h]@83
    uint v38;

    // Build AI array
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        Actor::MakeActorAIList_ODM();
    else
        Actor::MakeActorAIList_BLV();

    // Armageddon damage mechanic
    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR && pParty->armageddon_timer > 0)
        armageddonProgress();

    // Turn-based mode: return
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->AITurnBasedAction();
        return;
    }

    // this loops over all actors in background ai state
    for (uint i = 0; i < pActors.size(); ++i) {
        Actor *pActor = &pActors[i];
        ai_near_actors_targets_pid[i] = PID(OBJECT_Character, 0);

        // Skip actor if: Dead / Removed / Disabled / or in full ai state
        if (pActor->aiState == Dead || pActor->aiState == Removed ||
            pActor->aiState == Disabled || pActor->attributes & ACTOR_FULL_AI_STATE)
            continue;

        // Kill actor if HP == 0
        if (!pActor->currentHP && pActor->aiState != Dying)
            Actor::Die(i);

        // Kill buffs if expired
        for (ACTOR_BUFF_INDEX i : pActor->buffs.indices())
            if (i != ACTOR_BUFF_MASS_DISTORTION)
                pActor->buffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime());

        // If shrink expired: reset height
        if (pActor->buffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->height = pMonsterList->pMonsters[pActor->monsterInfo.uID - 1].uMonsterHeight;
            pActor->buffs[ACTOR_BUFF_SHRINK].Reset();
        }

        // If Charm still active: make actor friendly
        if (pActor->buffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
        } else if (pActor->buffs[ACTOR_BUFF_CHARM].Expired()) {
          // Else: reset hostilty
          pActor->monsterInfo.uHostilityType = pMonsterStats->pInfos[pActor->monsterInfo.uID].uHostilityType;
          pActor->buffs[ACTOR_BUFF_CHARM].Reset();
        }

        // If actor Paralyzed or Stoned: skip
        if (pActor->buffs[ACTOR_BUFF_PARALYZED].Active() || pActor->buffs[ACTOR_BUFF_STONED].Active())
            continue;

        // If actor is stunned: skip - vanilla bug that causes stunned background actors to recover to idle motions
        // Most apparent during armageddon spell, falling background actors will occasionally hover to perform action
        if (pActor->aiState == AIState::Stunned)
            continue;

        // Calculate RecoveryTime
        pActor->monsterInfo.uRecoveryTime = std::max(pActor->monsterInfo.uRecoveryTime - pEventTimer->uTimeElapsed, 0); // was pMiscTimer

        pActor->currentActionTime += pEventTimer->uTimeElapsed; // was pMiscTimer
        if (pActor->currentActionTime < pActor->currentActionLength)
            continue;

        if (pActor->aiState == Dying) {
            pActor->aiState = Dead;
        } else {
            if (pActor->aiState != Summoned) {
                Actor::AI_StandOrBored(i, PID(OBJECT_Character, 0), 256, nullptr);
                continue;
            }
            pActor->aiState = Standing;
        }

        pActor->currentActionTime = 0;
        pActor->currentActionLength = 0;
        pActor->UpdateAnimation();
    }

    // loops over for the actors in "full" ai state
    for (int v78 = 0; v78 < ai_arrays_size; ++v78) {
        uint actor_id = ai_near_actors_ids[v78];
        assert(actor_id < pActors.size());
        int actorPid = PID(OBJECT_Actor, actor_id);

        Actor *pActor = &pActors[actor_id];

        v47 = pActor->monsterInfo.uRecoveryTime * flt_debugrecmod3;

        Actor::_SelectTarget(actor_id, &ai_near_actors_targets_pid[actor_id], true);

        if (pActor->monsterInfo.uHostilityType && !ai_near_actors_targets_pid[actor_id])
            pActor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;

        target_pid = ai_near_actors_targets_pid[actor_id];
        target_pid_type = PID_TYPE(target_pid);

        float radiusMultiplier = target_pid_type == OBJECT_Actor ? 0.5 : 1.0;

        // v22 = pActor->uAIState;
        if (pActor->aiState == Dying || pActor->aiState == Dead || pActor->aiState == Removed ||
            pActor->aiState == Disabled || pActor->aiState == Summoned)
            continue;

        if (!pActor->currentHP)
            Actor::Die(actor_id);

        for (ACTOR_BUFF_INDEX i : pActor->buffs.indices())
            if (i != ACTOR_BUFF_MASS_DISTORTION)
                pActor->buffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime());

        if (pActor->buffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->height = pMonsterList->pMonsters[pActor->monsterInfo.uID - 1].uMonsterHeight;
            pActor->buffs[ACTOR_BUFF_SHRINK].Reset();
        }

        if (pActor->buffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
        } else if (pActor->buffs[ACTOR_BUFF_CHARM].Expired()) {
            pActor->monsterInfo.uHostilityType = pMonsterStats->pInfos[pActor->monsterInfo.uID].uHostilityType;
            pActor->buffs[ACTOR_BUFF_CHARM].Reset();
        }

        // If actor is summoned and buff expired: continue and set state to
        // Removed
        if (pActor->buffs[ACTOR_BUFF_SUMMONED].Expired()) {
            pActor->aiState = Removed;
            continue;
        }

        if (pActor->buffs[ACTOR_BUFF_STONED].Active() ||
            pActor->buffs[ACTOR_BUFF_PARALYZED].Active()) {
            continue;
        }

        pActor->monsterInfo.uRecoveryTime = std::max(0, pActor->monsterInfo.uRecoveryTime - pEventTimer->uTimeElapsed); // was pMiscTimer
        pActor->currentActionTime += pEventTimer->uTimeElapsed; // was pMiscTimer

        if (!pActor->ActorNearby())
            pActor->attributes |= ACTOR_NEARBY;

        AIDirection targetDirection;
        Actor::GetDirectionInfo(actorPid, target_pid, &targetDirection, 0);
        pDir = &targetDirection;
        AIState uAIState = pActor->aiState;

        // TODO(captainurist): this check makes no sense, it fails only for monsters that are:
        // stunned && non-friendly && recovering && far from target && don't have missile attack. Seriously?
        if (pActor->monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly ||
            pActor->monsterInfo.uRecoveryTime > 0 ||
            radiusMultiplier * 307.2 < pDir->uDistance ||
            uAIState != Pursuing && uAIState != Standing && uAIState != Tethered && uAIState != Fidgeting && !pActor->monsterInfo.uMissleAttack1Type ||
            uAIState != Stunned) {
            if (pActor->currentActionTime < pActor->currentActionLength) {
                continue;
            } else if (pActor->aiState == AttackingMelee) {
                pushMeleeAttack(actorPid, pActor->pos + Vec3i(0, 0, pActor->height / 2), pActor->special_ability_use_check(actor_id));
            } else if (pActor->aiState == AttackingRanged1) {
                Actor::AI_RangedAttack(actor_id, pDir, pActor->monsterInfo.uMissleAttack1Type, ABILITY_ATTACK1);  // light missile
            } else if (pActor->aiState == AttackingRanged2) {
                Actor::AI_RangedAttack(actor_id, pDir, pActor->monsterInfo.uMissleAttack2Type, ABILITY_ATTACK2);  // arrow
            } else if (pActor->aiState == AttackingRanged3) {
                Actor::AI_SpellAttack(actor_id, pDir, pActor->monsterInfo.uSpell1ID, ABILITY_SPELL1,
                                      pActor->monsterInfo.uSpellSkillAndMastery1);
            } else if (pActor->aiState == AttackingRanged4) {
                Actor::AI_SpellAttack(actor_id, pDir, pActor->monsterInfo.uSpell2ID, ABILITY_SPELL2,
                                      pActor->monsterInfo.uSpellSkillAndMastery2);
            }
        }

        int distanceToTarget = pDir->uDistance;

        int relationToTarget;
        if (pActor->monsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly) {
            if (target_pid_type == OBJECT_Actor) {
                relationToTarget = pFactionTable->relations[(pActor->monsterInfo.uID - 1) / 3 + 1][(pActors[PID_ID(target_pid)].monsterInfo.uID - 1) / 3 + 1];
            } else {
                relationToTarget = 4;
            }
            v38 = 0;
            if (relationToTarget == 2)
                v38 = 1024;
            else if (relationToTarget == 3)
                v38 = 2560;
            else if (relationToTarget == 4)
                v38 = 5120;
            if (relationToTarget >= 1 && relationToTarget <= 4 && distanceToTarget < v38 || relationToTarget == 1)
                pActor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Long;
        }

        // If actor afraid: flee or if out of range random move
        if (pActor->buffs[ACTOR_BUFF_AFRAID].Active()) {
            if (distanceToTarget >= 10240)
                Actor::AI_RandomMove(actor_id, target_pid, 1024, 0);
            else
                Actor::AI_Flee(actor_id, target_pid, 0, pDir);
            continue;
        }

        if (pActor->monsterInfo.uHostilityType == MonsterInfo::Hostility_Long &&
            target_pid) {
            if (pActor->monsterInfo.uAIType == 1) {
                if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                    Actor::AI_Stand(actor_id, target_pid, (pActor->monsterInfo.uRecoveryTime * flt_debugrecmod3), pDir);
                } else {
                    Actor::AI_Flee(actor_id, target_pid, 0, pDir);
                    continue;
                }
            }
            if (!(pActor->attributes & ACTOR_FLEEING)) {
                if (pActor->monsterInfo.uAIType == 2 || pActor->monsterInfo.uAIType == 3) {
                    if (pActor->monsterInfo.uAIType == 2)
                        v43 = pActor->monsterInfo.uHP * 0.2;
                    if (pActor->monsterInfo.uAIType == 3)
                        v43 = pActor->monsterInfo.uHP * 0.1;
                    v42 = pActor->currentHP;
                    if (v43 > v42 && distanceToTarget < 10240) {
                        Actor::AI_Flee(actor_id, target_pid, 0, pDir);
                        continue;
                    }
                }
            }

            v81 = distanceToTarget - pActor->radius;
            if (target_pid_type == OBJECT_Actor)
                v81 -= pActors[PID_ID(target_pid)].radius;
            if (v81 < 0)
                v81 = 0;
            // rand();
            pActor->attributes &= ~ACTOR_UNKNOW5;  // ~0x40000
            if (v81 < 5120) {
                v45 = pActor->special_ability_use_check(actor_id);
                if (v45 == ABILITY_ATTACK1) {
                    if (pActor->monsterInfo.uMissleAttack1Type) {
                        if (pActor->monsterInfo.uRecoveryTime <= 0) {
                            Actor::AI_MissileAttack1(actor_id, target_pid, pDir);
                        } else if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            if (radiusMultiplier * 307.2 > v81)
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            else
                                Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
                        }
                    } else {
                        if (v81 >= radiusMultiplier * 307.2) {
                            if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            } else if (v81 >= 1024) {  // monsters
                                Actor::AI_Pursue3(actor_id, target_pid, 0, pDir);
                            } else {
                                v70 = (radiusMultiplier * 307.2);
                                // monsters
                                // guard after player runs away
                                // follow player
                                Actor::AI_Pursue2(actor_id, target_pid, 0, pDir, v70);
                            }
                        } else if (pActor->monsterInfo.uRecoveryTime > 0) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            // monsters
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                } else if (v45 == ABILITY_SPELL1 || v45 == ABILITY_SPELL2) {
                    if (v45 == ABILITY_SPELL1)
                        v46 = pActor->monsterInfo.uSpell1ID;
                    else
                        v46 = pActor->monsterInfo.uSpell2ID;
                    if (v46) {
                        if (pActor->monsterInfo.uRecoveryTime <= 0) {
                            if (v45 == ABILITY_SPELL1)
                                Actor::AI_SpellAttack1(actor_id, target_pid, pDir);
                            else
                                Actor::AI_SpellAttack2(actor_id, target_pid, pDir);
                        } else if (radiusMultiplier * 307.2 > v81 || pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
                        }
                    } else {
                        // v45 == ABILITY_ATTACK2
                        if (v81 >= radiusMultiplier * 307.2) {
                            if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            } else if (v81 >= 1024) {
                                Actor::AI_Pursue3(actor_id, target_pid, 256, pDir);
                            } else {
                                v70 = (radiusMultiplier * 307.2);
                                Actor::AI_Pursue2(actor_id, target_pid, 0, pDir, v70);
                            }
                        } else if (pActor->monsterInfo.uRecoveryTime > 0) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                }
            }
        }

        if (pActor->monsterInfo.uHostilityType != MonsterInfo::Hostility_Long ||
            !target_pid || v81 >= 5120 || v45 != ABILITY_ATTACK2) {
            if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_SHORT) {
                Actor::AI_RandomMove(actor_id, 4, 1024, 0);
            } else if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_MEDIUM) {
                Actor::AI_RandomMove(actor_id, 4, 2560, 0);
            } else if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_LONG) {
                Actor::AI_RandomMove(actor_id, 4, 5120, 0);
            } else if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_FREE) {
                Actor::AI_RandomMove(actor_id, 4, 10240, 0);
            } else if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                Actor::GetDirectionInfo(actorPid, 4, &v72, 0);
                v58 = (pActor->monsterInfo.uRecoveryTime * flt_debugrecmod3);
                Actor::AI_Stand(actor_id, 4, v58, &v72);
            }
        } else if (!pActor->monsterInfo.uMissleAttack2Type) {
            if (v81 >= radiusMultiplier * 307.2) {
                if (pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                    Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                } else if (v81 >= 1024) {
                    Actor::AI_Pursue3(actor_id, target_pid, 256, pDir);
                } else {
                    v70 = (radiusMultiplier * 307.2);
                    Actor::AI_Pursue2(actor_id, target_pid, 0, pDir, v70);
                }
            } else if (pActor->monsterInfo.uRecoveryTime > 0) {
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            } else {
                Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
            }
        } else if (pActor->monsterInfo.uRecoveryTime > 0) {
            if (radiusMultiplier * 307.2 > v81 || pActor->monsterInfo.uMovementType == MONSTER_MOVEMENT_TYPE_STATIONARY)
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            else
                Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
        } else {
            Actor::AI_MissileAttack2(actor_id, target_pid, pDir);
        }
    }
}

bool Actor::isActorKilled(ACTOR_KILL_CHECK_POLICY policy, int param, int count) {
    int deadActors = 0;
    int totalActors = 0;

    switch (policy) {
      case KILL_CHECK_ANY:
        deadActors = Actor::searchDeadActors(&totalActors);
        break;
      case KILL_CHECK_GROUPID:
        deadActors = Actor::searchDeadActorsByGroup(&totalActors, param);
        break;
      case KILL_CHECK_MONSTERID:
        deadActors = Actor::searchDeadActorsByMonsterID(&totalActors, param);
        break;
      case KILL_CHECK_ACTORID:
        deadActors = Actor::searchDeadActorsByID(&totalActors, param);
        break;
      default:
        return false;
    }

    if (count) {
        return deadActors >= count;
    } else {
        return totalActors == deadActors;
    }
}

int Actor::searchDeadActorsByID(int *pTotalActors, int id) {
    *pTotalActors = 0;
    if (!!(pActors[id].attributes & ACTOR_UNKNOW7) == GetAlertStatus()) {
        *pTotalActors = 1;
        if (pActors[id].IsNotAlive()) {
            return 1;
        }
    }
    return 0;
}

int Actor::searchDeadActorsByGroup(int *pTotalActors, int group) {
    int result = 0, totalActors = 0;
    bool alert = GetAlertStatus();

    for (uint i = 0; i < pActors.size(); i++) {
        if (!!(pActors[i].attributes & ACTOR_UNKNOW7) == alert && pActors[i].group == group) {
            totalActors++;
            if (pActors[i].IsNotAlive()) {
                result++;
            }
        }
    }

    *pTotalActors = totalActors;
    return result;
}

int Actor::searchDeadActorsByMonsterID(int *pTotalActors, int monsterID) {
    int result = 0, totalActors = 0;
    bool alert = GetAlertStatus();

    for (uint i = 0; i < pActors.size(); i++) {
        if (!!(pActors[i].attributes & ACTOR_UNKNOW7) == alert && pActors[i].monsterInfo.uID == monsterID) {
            totalActors++;
            if (pActors[i].IsNotAlive()) {
                result++;
            }
        }
    }

    *pTotalActors = totalActors;
    return result;
}

int Actor::searchDeadActors(int *pTotalActors) {
    int result = 0, totalActors = 0;
    bool alert = GetAlertStatus();

    for (uint i = 0; i < pActors.size(); i++) {
        if (!!(pActors[i].attributes & ACTOR_UNKNOW7) == alert) {
            totalActors++;
            if (pActors[i].IsNotAlive()) {
                result++;
            }
        }
    }

    *pTotalActors = totalActors;
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
    if (pParty->isPartyGood()) good = true;
    if (pParty->isPartyEvil()) evil = true;

    ai_near_actors_targets_pid.fill(0);

    for (uint i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];

        if (actor->CanAct() || actor->aiState == Disabled) {
            actor->pos.x = actor->initialPosition.x;
            actor->pos.y = actor->initialPosition.y;
            actor->pos.z = actor->initialPosition.z;
            actor->currentHP = actor->monsterInfo.uHP;
            if (actor->aiState != Disabled) {
                Actor::AI_Stand(i, ai_near_actors_targets_pid[i],
                                actor->monsterInfo.uRecoveryTime, 0);
            }
        }

        actor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;

        if (!bCelestia || good)
            if (!bPit || evil)
                if (actor->IsPeasant()) actor->ResetAggressor();  // ~0x80000

        actor->ResetHasItem();  // ~0x800000
        if (actor->attributes & ACTOR_UNKNOW9)
            Actor::_4031C1_update_job_never_gets_called(i, pParty->uCurrentHour,
                                                        1);
    }
}
//----- (00439474) --------------------------------------------------------
void Actor::DamageMonsterFromParty(signed int a1, unsigned int uActorID_Monster,
                                   Vec3i *pVelocity) {
    SpriteObject *projectileSprite;  // ebx@1
    Actor *pMonster;                 // esi@7
    int extraRecoveryTime;           // qax@125
    uint16_t v43{};            // ax@132
    uint16_t v45{};            // ax@132
    // uint64_t v46; // [sp+Ch] [bp-60h]@6
    int skillLevel = 0;                    // [sp+44h] [bp-28h]@1
    bool IsAdditionalDamagePossible;  // [sp+50h] [bp-1Ch]@1
    int v61;                          // [sp+58h] [bp-14h]@1
    bool isLifeStealing;              // [sp+5Ch] [bp-10h]@1
    int uDamageAmount;                // [sp+60h] [bp-Ch]@1
    DAMAGE_TYPE attackElement;        // [sp+64h] [bp-8h]@27

    projectileSprite = 0;
    uDamageAmount = 0;
    v61 = 0;
    IsAdditionalDamagePossible = false;
    isLifeStealing = 0;
    if (PID_TYPE(a1) == OBJECT_Item) {
        projectileSprite = &pSpriteObjects[PID_ID(a1)];
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        a1 = projectileSprite->spell_caster_pid;
    }
    if (PID_TYPE(a1) != OBJECT_Character) return;

    assert(PID_ID(abs(a1)) < 4);
    Character *character = &pParty->pCharacters[PID_ID(a1)];
    pMonster = &pActors[uActorID_Monster];
    if (pMonster->IsNotAlive()) return;

    pMonster->attributes |= ACTOR_NEARBY | ACTOR_ACTIVE;
    if (pMonster->aiState == Fleeing) pMonster->attributes |= ACTOR_FLEEING;
    bool hit_will_stun = false, hit_will_paralyze = false;
    if (!projectileSprite) {
        int main_hand_idx = character->pEquipment.uMainHand;
        IsAdditionalDamagePossible = true;
        if (character->HasItemEquipped(ITEM_SLOT_MAIN_HAND)) {
            CharacterSkillType main_hand_skill = character->GetMainHandItem()->GetPlayerSkillType();
            CharacterSkillMastery main_hand_mastery = character->getSkillValue(main_hand_skill).mastery();
            switch (main_hand_skill) {
                case CHARACTER_SKILL_STAFF:
                    if (main_hand_mastery >= CHARACTER_SKILL_MASTERY_MASTER) {
                        if (grng->random(100) < character->getActualSkillValue(CHARACTER_SKILL_STAFF).level())
                            hit_will_stun = true;
                    }
                    break;

                case CHARACTER_SKILL_MACE:
                    if (main_hand_mastery >= CHARACTER_SKILL_MASTERY_MASTER) {
                        if (grng->random(100) < character->getActualSkillValue(CHARACTER_SKILL_MACE).level())
                            hit_will_stun = true;
                    }
                    if (main_hand_mastery >= CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                        if (grng->random(100) < character->getActualSkillValue(CHARACTER_SKILL_MACE).level())
                            hit_will_paralyze = true;
                    }
                    break;

                default:
                    break;
            }
        }
        attackElement = DMGT_PHISYCAL;
        uDamageAmount = character->CalculateMeleeDamageTo(false, false, pMonster->monsterInfo.uID);
        if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
            character->playReaction(SPEECH_ATTACK_MISS);
            return;
        }
    } else {
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        if (projectileSprite->uSpellID != SPELL_DARK_SOULDRINKER) {
            int d1 = abs(pParty->pos.x - projectileSprite->vPosition.x);
            int d2 = abs(pParty->pos.y - projectileSprite->vPosition.y);
            int d3 = abs(pParty->pos.z - projectileSprite->vPosition.z);
            v61 = int_get_vector_length(d1, d2, d3);

            if (v61 >= 5120 && !(pMonster->attributes & ACTOR_FULL_AI_STATE))  // 0x400
                return;
            else if (v61 >= 2560)
                v61 = 2;
            else
                v61 = 1;
        }

        switch (projectileSprite->uSpellID) {
            case SPELL_LASER_PROJECTILE:
                // TODO: should be changed to GetActual* equivalents?
                v61 = 1;
                if (character->getSkillValue(CHARACTER_SKILL_BLASTER).mastery() >= CHARACTER_SKILL_MASTERY_MASTER)
                    skillLevel = character->getSkillValue(CHARACTER_SKILL_BLASTER).level();
                attackElement = DMGT_PHISYCAL;
                uDamageAmount = character->CalculateMeleeDamageTo(true, true, 0);
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return;
                }
                break;
            case SPELL_101:
                attackElement = DMGT_FIRE;
                uDamageAmount = character->CalculateRangedDamageTo(0);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = true;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return;
                }
                break;
            case SPELL_EARTH_BLADES:
                skillLevel = 5 * projectileSprite->spell_level;
                attackElement =
                    (DAMAGE_TYPE)character->GetSpellSchool(SPELL_EARTH_BLADES);
                uDamageAmount = CalcSpellDamage(
                    SPELL_EARTH_BLADES, projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->currentHP);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = false;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return;
                }
                break;
            case SPELL_EARTH_STUN:
                uDamageAmount = 0;
                attackElement = DMGT_PHISYCAL;
                hit_will_stun = 1;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return;
                }
                break;
            case SPELL_BOW_ARROW:
                attackElement = DMGT_PHISYCAL;
                uDamageAmount = character->CalculateRangedDamageTo(
                    pMonster->word_000086_some_monster_id);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount /= 2;
                IsAdditionalDamagePossible = true;
                if (projectileSprite->containing_item.uItemID != ITEM_NULL &&
                    projectileSprite->containing_item.special_enchantment == ITEM_ENCHANTMENT_OF_CARNAGE) {
                    attackElement = DMGT_FIRE;
                } else if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return;
                }
                break;

            default:
                attackElement = (DAMAGE_TYPE)character->GetSpellSchool(
                    projectileSprite->uSpellID);
                IsAdditionalDamagePossible = false;
                uDamageAmount = CalcSpellDamage(
                    projectileSprite->uSpellID,
                    projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->currentHP);
                break;
        }
    }

    if (character->IsWeak()) uDamageAmount /= 2;
    if (pMonster->buffs[ACTOR_BUFF_STONED].Active()) uDamageAmount = 0;
    v61 = pMonster->CalcMagicalDamageToActor(attackElement, uDamageAmount);
    if (!projectileSprite && character->IsUnarmed() &&
        character->pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].Active()) {
        v61 += pMonster->CalcMagicalDamageToActor(
            DMGT_BODY,
            character->pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].power);
    }
    uDamageAmount = v61;
    if (IsAdditionalDamagePossible) {
        if (projectileSprite) {
            skillLevel =
                projectileSprite->containing_item._439DF3_get_additional_damage(
                    &attackElement, &isLifeStealing);
            if (isLifeStealing && pMonster->currentHP > 0) {
                character->health += v61 / 5;
                if (character->health > character->GetMaxHealth())
                    character->health = character->GetMaxHealth();
            }
            uDamageAmount +=
                pMonster->CalcMagicalDamageToActor(attackElement, skillLevel);
        } else {
            for (ITEM_SLOT i : {ITEM_SLOT_OFF_HAND, ITEM_SLOT_MAIN_HAND}) {
                if (character->HasItemEquipped(i)) {
                    ItemGen *item;
                    if (i == ITEM_SLOT_OFF_HAND)
                        item = character->GetOffHandItem();
                    else
                        item = character->GetMainHandItem();
                    skillLevel = item->_439DF3_get_additional_damage(&attackElement,
                                                             &isLifeStealing);
                    if (isLifeStealing && pMonster->currentHP > 0) {
                        character->health += v61 / 5;
                        if (character->health > character->GetMaxHealth())
                            character->health = character->GetMaxHealth();
                    }
                    uDamageAmount +=
                        pMonster->CalcMagicalDamageToActor(attackElement, skillLevel);
                }
            }
        }
    }
    pMonster->currentHP -= uDamageAmount;
    if (uDamageAmount == 0 && !hit_will_stun) {
        character->playReaction(SPEECH_ATTACK_MISS);
        return;
    }
    if (pMonster->currentHP > 0) {
        Actor::AI_Stun(uActorID_Monster, a1, 0);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (engine->config->settings.ShowHits.value()) {
            if (projectileSprite)
                GameUI_SetStatusBar(
                    LSTR_FMT_S_SHOOTS_S_FOR_U,
                    character->name.c_str(),
                    pMonster->name.c_str(),
                    uDamageAmount
                );
            else
                GameUI_SetStatusBar(
                    LSTR_FMT_S_HITS_S_FOR_U,
                    character->name.c_str(),
                    pMonster->name.c_str(),
                    uDamageAmount
                );
        }
    } else {
        Actor::Die(uActorID_Monster);
        Actor::ApplyFineForKillingPeasant(uActorID_Monster);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (pMonster->monsterInfo.uExp)
            pParty->GivePartyExp(
                pMonsterStats->pInfos[pMonster->monsterInfo.uID].uExp);
        CharacterSpeech speech = SPEECH_ATTACK_HIT;
        if (vrng->random(100) < 20) {
            speech = pMonster->monsterInfo.uHP >= 100 ? SPEECH_KILL_STRONG_ENEMY : SPEECH_KILL_WEAK_ENEMY;
        }
        character->playReaction(speech);
        if (engine->config->settings.ShowHits.value()) {
            GameUI_SetStatusBar(
                LSTR_FMT_S_INFLICTS_U_KILLING_S,
                character->name.c_str(),
                uDamageAmount,
                pMonster->name.c_str()
            );
        }
    }
    if (pMonster->buffs[ACTOR_BUFF_PAIN_REFLECTION].Active() && uDamageAmount != 0)
        character->receiveDamage(uDamageAmount, attackElement);
    int knockbackValue = 20 * v61 / (signed int)pMonster->monsterInfo.uHP;
    if ((character->GetSpecialItemBonus(ITEM_ENCHANTMENT_OF_FORCE) ||
         hit_will_stun) && pMonster->DoesDmgTypeDoDamage(DMGT_EARTH)) {
        extraRecoveryTime = 20;
        knockbackValue = 10;
        if (!pParty->bTurnBasedModeOn)
            extraRecoveryTime = (int)(debug_combat_recovery_mul * flt_debugrecmod3 * 20.0);
        pMonster->monsterInfo.uRecoveryTime += extraRecoveryTime;
        if (engine->config->settings.ShowHits.value()) {
            GameUI_SetStatusBar(
                LSTR_FMT_S_STUNS_S,
                character->name,
                pMonster->name
            );
        }
    }
    if (hit_will_paralyze && pMonster->CanAct() &&
        pMonster->DoesDmgTypeDoDamage(DMGT_EARTH)) {
        CombinedSkillValue maceSkill = character->getActualSkillValue(CHARACTER_SKILL_MACE);
        GameTime v46 = GameTime(0, maceSkill.level());  // ??
        pMonster->buffs[ACTOR_BUFF_PARALYZED].Apply((pParty->GetPlayingTime() + v46), maceSkill.mastery(), 0, 0, 0);
        if (engine->config->settings.ShowHits.value()) {
            GameUI_SetStatusBar(
                LSTR_FMT_S_PARALYZES_S,
                character->name,
                pMonster->name
            );
        }
    }
    if (knockbackValue > 10) knockbackValue = 10;
    if (!MonsterStats::BelongsToSupertype(pMonster->monsterInfo.uID,
                                          MONSTER_SUPERTYPE_TREANT)) {
        pVelocity->x = fixpoint_mul(knockbackValue, pVelocity->x);
        pVelocity->y = fixpoint_mul(knockbackValue, pVelocity->y);
        pVelocity->z = fixpoint_mul(knockbackValue, pVelocity->z);
        pMonster->speed.x = 50 * (short)pVelocity->x;
        pMonster->speed.y = 50 * (short)pVelocity->y;
        pMonster->speed.z = 50 * (short)pVelocity->z;
    }
    Actor::AddOnDamageOverlay(uActorID_Monster, 1, v61);
}

//----- (004BBF61) --------------------------------------------------------
void Actor::Arena_summon_actor(int monster_id, int x, int y, int z) {
    Actor *actor = AllocateActor(true);
    if (!actor)
        return;

    int v16 = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        v16 = pIndoor->GetSector(x, y, z);

    actor->name = pMonsterStats->pInfos[monster_id].pName;
    actor->currentHP = (short)pMonsterStats->pInfos[monster_id].uHP;
    actor->monsterInfo = pMonsterStats->pInfos[monster_id];
    actor->word_000086_some_monster_id = monster_id;
    actor->radius = pMonsterList->pMonsters[monster_id - 1].uMonsterRadius;
    actor->height = pMonsterList->pMonsters[monster_id - 1].uMonsterHeight;
    actor->moveSpeed = pMonsterList->pMonsters[monster_id - 1].uMovementSpeed;
    actor->initialPosition.x = x;
    actor->pos.x = x;
    actor->attributes |= ACTOR_AGGRESSOR;
    actor->monsterInfo.uTreasureType = 0;
    actor->monsterInfo.uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    actor->monsterInfo.uTreasureDiceSides = 0;
    actor->monsterInfo.uTreasureDiceRolls = 0;
    actor->monsterInfo.uTreasureDropChance = 0;
    actor->initialPosition.y = y;
    actor->pos.y = y;
    actor->initialPosition.z = z;
    actor->pos.z = z;
    actor->tetherDistance = 256;
    actor->sectorId = v16;
    actor->group = 1;
    actor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Long;
    actor->PrepareSprites(0);
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
}

//----- (00426E10) --------------------------------------------------------
int stru319::which_player_to_attack(Actor *pActor) {
    signed int v2;         // ebx@1
    int v22;               // [sp+8h] [bp-140h]@3
    int Victims_list[60] {};  // [sp+48h] [bp-100h]@48
    int for_sex;           // [sp+13Ch] [bp-Ch]@1
    int for_race;          // [sp+140h] [bp-8h]@1
    int for_class;         // [sp+144h] [bp-4h]@1

    for_class = -1;
    for_race = -1;
    for_sex = -1;
    v2 = 0;
    if (pActor->monsterInfo.uAttackPreference) {
        for (uint i = 0; i < 16; i++) {
            v22 = pActor->monsterInfo.uAttackPreference & (1 << i);
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
                for (int j = 0; j < pParty->pCharacters.size(); ++j) {
                    bool flag = 0;
                    if (for_class != -1 && for_class == pParty->pCharacters[j].classType) {
                        flag = true;
                    }
                    if (for_sex != -1 && for_sex == pParty->pCharacters[j].uSex) {
                        flag = true;
                    }
                    if (for_race != -1 && for_race == pParty->pCharacters[j].GetRace()) {
                        flag = true;
                    }
                    if (flag == true) {
                        if (pParty->pCharacters[j].conditions.HasNone({CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD,
                                                                    CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
                            Victims_list[v2++] = j;
                        }
                    }
                }
            }
        }
        if (v2) return Victims_list[grng->random(v2)];
    }
    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        if (pParty->pCharacters[i].conditions.HasNone({CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD,
                                                    CONDITION_PETRIFIED, CONDITION_ERADICATED}))
            Victims_list[v2++] = i;
    }
    if (v2)
        return Victims_list[grng->random(v2)];
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
int stru319::FindClosestActor(int pick_depth, int a3 /*Relates to targeting/not targeting allies?*/, int target_undead) {
    int v4;       // edi@1
    stru319 *v5;  // esi@1
    VisSelectFlags select_flags;       // eax@2
    int v7;       // eax@4
    //  int result; // eax@5
    //  int *v9; // edx@8
    //  signed int v10; // ebx@10
    //  int v11; // edi@11
    // Actor *v12; // esi@12
    // uint16_t v13; // ax@12
    //  int v14; // eax@22
    // char v15; // zf@30
    //  int v16; // esi@32
    //  int v17; // ecx@34
    //  stru319 *v18; // eax@39
    //  int v19; // edx@39
    //  int v20; // ecx@41
    //  uint16_t v21; // ax@42
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
        select_flags = (a3 != 0) ? VisSelectFlags_1 : None;
        if (target_undead) select_flags |= TargetUndead;
        v7 = vis->PickClosestActor(OBJECT_Actor, pick_depth, static_cast<VisSelectFlags>(select_flags), 657456, -1);
        if (v7 != -1)
            return (uint16_t)v7;
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
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) distance = 2560;

    for (uint i = 0; i < pActors.size(); ++i) {
        for_x = abs(pActors[i].pos.x - pParty->pos.x);
        for_y = abs(pActors[i].pos.y - pParty->pos.y);
        for_z = abs(pActors[i].pos.z - pParty->pos.z);
        if (int_get_vector_length(for_x, for_y, for_z) < distance) {
            if (pActors[i].aiState != Dead) {
                if (pActors[i].aiState != Dying &&
                    pActors[i].aiState != Removed &&
                    pActors[i].aiState != Disabled &&
                    pActors[i].aiState != Summoned &&
                    (pActors[i].ActorEnemy() ||
                     pActors[i].GetActorsRelation(0)))
                    return true;
            }
        }
    }
    return false;
}


void StatusBarItemFound(int num_gold_found, const std::string &item_unidentified_name) {
    if (num_gold_found != 0) {
        GameUI_SetStatusBar(
            LSTR_FMT_YOU_FOUND_GOLD_AND_ITEM,
            num_gold_found,
            item_unidentified_name.c_str()
        );
    } else {
        GameUI_SetStatusBar(
            LSTR_FMT_YOU_FOUND_ITEM,
            item_unidentified_name.c_str()
        );
    }
}


//----- (00426A5A) --------------------------------------------------------
void Actor::LootActor() {
    ItemGen Dst;         // [sp+Ch] [bp-2Ch]@1
    bool itemFound;      // [sp+30h] [bp-8h]@1

    pParty->placeHeldItemInInventoryOrDrop();
    Dst.Reset();
    itemFound = false;
    int foundGold = 0;
    if (!ActorHasItem()) {
        foundGold = grng->randomDice(this->monsterInfo.uTreasureDiceRolls, this->monsterInfo.uTreasureDiceSides);
        if (foundGold) {
            pParty->partyFindsGold(foundGold, GOLD_RECEIVE_SHARE);
        }
    } else {
        if (this->items[3].isGold()) {
            foundGold = this->items[3].special_enchantment;
            this->items[3].Reset();
            if (foundGold) {
                pParty->partyFindsGold(foundGold, GOLD_RECEIVE_SHARE);
            }
        }
    }
    if (this->carriedItemId != ITEM_NULL) {
        Dst.Reset();
        Dst.uItemID = this->carriedItemId;

        StatusBarItemFound(foundGold, pItemTable->pItems[Dst.uItemID].pUnidentifiedName);

        if (Dst.isWand()) {
            Dst.uNumCharges = grng->random(6) + Dst.GetDamageMod() + 1;
            Dst.uMaxCharges = Dst.uNumCharges;
        }
        if (Dst.isPotion() && Dst.uItemID != ITEM_POTION_BOTTLE) {
            Dst.uEnchantmentType = 2 * grng->random(4) + 2;
        }
        pItemTable->SetSpecialBonus(&Dst);
        if (!pParty->addItemToParty(&Dst)) {
            pParty->setHoldingItem(&Dst);
        }
        this->carriedItemId = ITEM_NULL;
        if (this->items[0].uItemID != ITEM_NULL) {
            if (!pParty->addItemToParty(&this->items[0])) {
                pParty->placeHeldItemInInventoryOrDrop();
                pParty->setHoldingItem(&this->items[0]);
            }
            this->items[0].Reset();
        }
        if (this->items[1].uItemID != ITEM_NULL) {
            if (!pParty->addItemToParty(&this->items[1])) {
                pParty->placeHeldItemInInventoryOrDrop();
                pParty->setHoldingItem(&this->items[1]);
            }
            this->items[1].Reset();
        }
        this->Remove();
        return;
    }
    if (this->ActorHasItem()) {
        if (this->items[3].uItemID != ITEM_NULL) {
            Dst = this->items[3];
            this->items[3].Reset();

            StatusBarItemFound(foundGold, pItemTable->pItems[Dst.uItemID].pUnidentifiedName);

            if (!pParty->addItemToParty(&Dst)) {
                pParty->setHoldingItem(&Dst);
            }
            itemFound = true;
        }
    } else {
        if (grng->random(100) < this->monsterInfo.uTreasureDropChance && this->monsterInfo.uTreasureLevel != ITEM_TREASURE_LEVEL_INVALID) {
            pItemTable->generateItem(this->monsterInfo.uTreasureLevel, this->monsterInfo.uTreasureType, &Dst);

            StatusBarItemFound(foundGold, pItemTable->pItems[Dst.uItemID].pUnidentifiedName);

            if (!pParty->addItemToParty(&Dst)) {
                pParty->setHoldingItem(&Dst);
            }
            itemFound = true;
        }
    }
    if (this->items[0].uItemID != ITEM_NULL) {
        if (!pParty->addItemToParty(&this->items[0])) {
            pParty->placeHeldItemInInventoryOrDrop();
            pParty->setHoldingItem(&this->items[0]);
            itemFound = true;
        }
        this->items[0].Reset();
    }
    if (this->items[1].uItemID != ITEM_NULL) {
        if (!pParty->addItemToParty(&this->items[1])) {
            pParty->placeHeldItemInInventoryOrDrop();
            pParty->setHoldingItem(&this->items[1]);
            itemFound = true;
        }
        this->items[1].Reset();
    }
    if (!itemFound || grng->random(100) < 90) {  // for repeatedly get gold and item
        this->Remove();
    }
}

//----- (00427102) --------------------------------------------------------
bool Actor::_427102_IsOkToCastSpell(SPELL_TYPE spell) {
    switch (spell) {
        case SPELL_BODY_POWER_CURE: {
            if (this->currentHP >= this->monsterInfo.uHP) return false;
            return true;
        }

        case SPELL_LIGHT_DISPEL_MAGIC: {
            for (SpellBuff &buff : pParty->pPartyBuffs) {
                if (buff.Active()) {
                    return true;
                }
            }
            for (Character &character : pParty->pCharacters) {
                for (SpellBuff &buff : character.pCharacterBuffs) {
                    if (buff.Active()) {
                        return true;
                    }
                }
            }
            return false;
        }

        case SPELL_LIGHT_DAY_OF_PROTECTION:
            return this->buffs[ACTOR_BUFF_DAY_OF_PROTECTION].Inactive();
        case SPELL_LIGHT_HOUR_OF_POWER:
            return this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Inactive();
        case SPELL_DARK_PAIN_REFLECTION:
            return this->buffs[ACTOR_BUFF_PAIN_REFLECTION].Inactive();
        case SPELL_BODY_HAMMERHANDS:
            return this->buffs[ACTOR_BUFF_HAMMERHANDS].Inactive();
        case SPELL_FIRE_HASTE:
            return this->buffs[ACTOR_BUFF_HASTE].Inactive();
        case SPELL_AIR_SHIELD:
            return this->buffs[ACTOR_BUFF_SHIELD].Inactive();
        case SPELL_EARTH_STONESKIN:
            return this->buffs[ACTOR_BUFF_STONESKIN].Inactive();
        case SPELL_SPIRIT_BLESS:
            return this->buffs[ACTOR_BUFF_BLESS].Inactive();
        case SPELL_SPIRIT_FATE:
            return this->buffs[ACTOR_BUFF_FATE].Inactive();
        case SPELL_SPIRIT_HEROISM:
            return this->buffs[ACTOR_BUFF_HEROISM].Inactive();
        default:
            return true;
    }
}

//----- (0042704B) --------------------------------------------------------
ABILITY_INDEX Actor::special_ability_use_check(int a2) {
    if (this->monsterInfo.uSpecialAbilityType == 2 && this->monsterInfo.uSpecialAbilityDamageDiceBonus < 3 && grng->random(100) < 5)
        this->SummonMinion(a2);

    bool okToCastSpell1 = this->_427102_IsOkToCastSpell(this->monsterInfo.uSpell1ID);
    bool okToCastSpell2 = this->_427102_IsOkToCastSpell(this->monsterInfo.uSpell2ID);
    if (okToCastSpell1 && this->monsterInfo.uSpell1UseChance && grng->random(100) < this->monsterInfo.uSpell1UseChance)
        return ABILITY_SPELL1;
    if (okToCastSpell2 && this->monsterInfo.uSpell2UseChance && grng->random(100) < this->monsterInfo.uSpell2UseChance)
        return ABILITY_SPELL2;
    if (this->monsterInfo.uAttack2Chance && grng->random(100) < this->monsterInfo.uAttack2Chance)
        return ABILITY_ATTACK2;
    return ABILITY_ATTACK1;
}

//----- (004273BB) --------------------------------------------------------
bool Actor::_4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4) {
    signed int v6;   // ebx@1
    signed int v7;   // esi@1
    int armorSum;    // ebx@10
    signed int a2a;  // [sp+18h] [bp+Ch]@1

    v6 = defender->monsterInfo.uAC;
    v7 = 0;
    a2a = 0;
    if (defender->buffs[ACTOR_BUFF_SOMETHING_THAT_HALVES_AC].Active())
        v6 /= 2;
    if (defender->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v7 = defender->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
    if (defender->buffs[ACTOR_BUFF_STONESKIN].Active() &&
        defender->buffs[ACTOR_BUFF_STONESKIN].power > v7)
        v7 = defender->buffs[ACTOR_BUFF_STONESKIN].power;
    armorSum = v7 + v6;
    if (this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        a2a = this->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
    if (this->buffs[ACTOR_BUFF_BLESS].Active() &&
        this->buffs[ACTOR_BUFF_BLESS].power > a2a)
        a2a = this->buffs[ACTOR_BUFF_BLESS].power;
    if (this->buffs[ACTOR_BUFF_FATE].Active()) {
        a2a += this->buffs[ACTOR_BUFF_FATE].power;
        this->buffs[ACTOR_BUFF_FATE].Reset();
    }
    return grng->random(armorSum + 2 * this->monsterInfo.uLevel + 10) + a2a + 1 >
           armorSum + 5;
}

//----- (004274AD) --------------------------------------------------------
bool Actor::ActorHitOrMiss(Character *pPlayer) {
    signed int v3;  // edi@1
    signed int v4;  // esi@8
    int v5;         // esi@8

    v3 = 0;
    if (this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v3 = this->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
    if (this->buffs[ACTOR_BUFF_BLESS].Active() &&
        this->buffs[ACTOR_BUFF_BLESS].power > v3)
        v3 = this->buffs[ACTOR_BUFF_BLESS].power;
    if (this->buffs[ACTOR_BUFF_FATE].Active()) {
        v3 += this->buffs[ACTOR_BUFF_FATE].power;
        this->buffs[ACTOR_BUFF_FATE].Reset();
    }
    v4 = pPlayer->GetActualAC() + 2 * this->monsterInfo.uLevel + 10;
    v5 = grng->random(v4) + 1;
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
    if (this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        v5 = this->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
    switch (dmgType) {
        case DMGT_FIRE:
            v6 = this->monsterInfo.uResFire;
            v4 = v5;
            break;
        case DMGT_ELECTR:
            v6 = this->monsterInfo.uResAir;
            v4 = v5;
            break;
        case DMGT_COLD:
            v6 = this->monsterInfo.uResWater;
            v4 = v5;
            break;
        case DMGT_EARTH:
            v6 = this->monsterInfo.uResEarth;
            v4 = v5;
            break;
        case DMGT_PHISYCAL:
            v6 = this->monsterInfo.uResPhysical;
            break;
        case DMGT_SPIRIT:
            v6 = this->monsterInfo.uResSpirit;
            break;
        case DMGT_MIND:
            v6 = this->monsterInfo.uResMind;
            v4 = v5;
            break;
        case DMGT_BODY:
            v6 = this->monsterInfo.uResBody;
            v4 = v5;
            break;
        case DMGT_LIGHT:
            v6 = this->monsterInfo.uResLight;
            break;
        case DMGT_DARK:
            v6 = this->monsterInfo.uResDark;
            break;
        default:
            v6 = 0;
            break;
    }
    if (v6 < 200) {
        v8 = v4 + v6 + 30;
        for (int i = 0; i < 4; i++) {
            if (grng->random(v8) < 30) break;
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

    switch (uType) {
        case DMGT_FIRE:
            resist = this->monsterInfo.uResFire;
            break;
        case DMGT_ELECTR:
            resist = this->monsterInfo.uResAir;
            break;
        case DMGT_COLD:
            resist = this->monsterInfo.uResWater;
            break;
        case DMGT_EARTH:
            resist = this->monsterInfo.uResEarth;
            break;
        case DMGT_PHISYCAL:
            resist = this->monsterInfo.uResPhysical;
            break;
        case DMGT_SPIRIT:
            resist = this->monsterInfo.uResSpirit;
            break;
        case DMGT_MIND:
            resist = this->monsterInfo.uResMind;
        case DMGT_BODY:
            resist = this->monsterInfo.uResBody;
            break;
        case DMGT_LIGHT:
            resist = this->monsterInfo.uResLight;
            break;
        case DMGT_DARK:
            resist = this->monsterInfo.uResDark;
            break;
        default:
            return true;
    }
    if (resist < 200)
        return grng->random((this->monsterInfo.uLevel >> 2) + resist + 30) < 30;
    else
        return false;
}

//----- (00448A98) --------------------------------------------------------
void toggleActorGroupFlag(unsigned int uGroupID, ActorAttribute uFlag,
                          bool bValue) {
    if (uGroupID) {
        if (bValue) {
            for (uint i = 0; i < (unsigned int)pActors.size(); ++i) {
                if (pActors[i].group == uGroupID) {
                    pActors[i].attributes |= uFlag;
                    if (uFlag == ACTOR_UNKNOW11) {
                        pActors[i].aiState = Disabled;
                        pActors[i].UpdateAnimation();
                    }
                }
            }
        } else {
            for (uint i = 0; i < (unsigned int)pActors.size(); ++i) {
                if (pActors[i].group == uGroupID) {
                    if (uFlag == ACTOR_UNKNOW11) {
                        if (pActors[i].aiState != Dead) {
                            if (pActors[i].aiState != Dying &&
                                pActors[i].aiState != Removed)
                                pActors[i].aiState = Standing;
                        }
                    }
                    pActors[i].attributes &= ~uFlag;
                }
            }
        }
    }
}

//----- (004014E6) --------------------------------------------------------
void Actor::MakeActorAIList_ODM() {
    std::vector<std::pair<int, int>> activeActorsDistances; // pair<id, distance>

    pParty->uFlags &= ~PARTY_FLAGS_1_ALERT_RED_OR_YELLOW;

    for (Actor &actor : pActors) {
        actor.ResetFullAiState();
        if (!actor.CanAct()) {
            actor.ResetActive();
            continue;
        }

        int delta_x = abs(pParty->pos.x - actor.pos.x);
        int delta_y = abs(pParty->pos.y - actor.pos.y);
        int delta_z = abs(pParty->pos.z - actor.pos.z);

        int distance = int_get_vector_length(delta_x, delta_y, delta_z) - actor.radius;
        if (distance < 0)
            distance = 0;

        if (distance < 5632) {
            actor.ResetHostile();
            if (actor.ActorEnemy() || actor.GetActorsRelation(0)) {
                actor.attributes |= ACTOR_HOSTILE;
                if (distance < 5120)
                    pParty->SetYellowAlert();
                if (distance < 307)
                    pParty->SetRedAlert();
            }
            actor.attributes |= ACTOR_ACTIVE;
            activeActorsDistances.push_back({actor.id, distance});
        } else {
            actor.ResetActive();
        }
    }

    // sort active actors by distance
    // use stable_sort to make tests work across all platforms
    std::stable_sort(activeActorsDistances.begin(), activeActorsDistances.end(), [] (std::pair<int, int> a, std::pair<int, int> b) { return a.second < b.second; });

    // and takes nearest 30
    for (int i = 0; (i < 30) && (i < activeActorsDistances.size()); i++) {
        ai_near_actors_ids[i] = activeActorsDistances[i].first;
        pActors[ai_near_actors_ids[i]].attributes |= ACTOR_FULL_AI_STATE;
    }

    ai_arrays_size = std::min(30, (int)activeActorsDistances.size());
}

//----- (004016FA) --------------------------------------------------------
int Actor::MakeActorAIList_BLV() {
    std::vector<std::pair<int, int>> activeActorsDistances; // pair<id, distance>
    std::vector<int> pickedActorIds;

    // reset party alert level
    pParty->uFlags &= ~PARTY_FLAGS_1_ALERT_RED_OR_YELLOW;

    // find actors that are in range and can act
    for (Actor &actor : pActors) {
        actor.ResetFullAiState();
        if (!actor.CanAct()) {
            actor.ResetActive();
            continue;
        }

        int delta_x = abs(pParty->pos.x - actor.pos.x);
        int delta_y = abs(pParty->pos.y - actor.pos.y);
        int delta_z = abs(pParty->pos.z - actor.pos.z);

        int distance = int_get_vector_length(delta_x, delta_y, delta_z) - actor.radius;
        if (distance < 0)
            distance = 0;

        // actor is in range
        if (distance < 10240) {
            actor.ResetHostile();
            if (actor.ActorEnemy() || actor.GetActorsRelation(0)) {
                actor.attributes |= ACTOR_HOSTILE;
                if (!(pParty->GetRedAlert()) && (double)distance < 307.2)
                    pParty->SetRedAlert();
                if (!(pParty->GetYellowAlert()) && distance < 5120)
                    pParty->SetYellowAlert();
            }
            activeActorsDistances.push_back({actor.id, distance});
        } else {
            // otherwise idle
            actor.ResetActive();
        }
    }

    // sort active actors by distance
    // use stable_sort to make tests work across all platforms
    std::stable_sort(activeActorsDistances.begin(), activeActorsDistances.end(), [] (std::pair<int, int> a, std::pair<int, int> b) { return a.second < b.second; });

    // checks nearby actors can detect player and take nearest 30
    for (const auto &[actorId, _] : activeActorsDistances) {
        if (pActors[actorId].ActorNearby() || Detect_Between_Objects(PID(OBJECT_Actor, actorId), PID(OBJECT_Character, 0))) {
            pActors[actorId].attributes |= ACTOR_NEARBY;
            pickedActorIds.push_back(actorId);
            if (pickedActorIds.size() >= 30) {
                break;
            }
        }
    }

    // add any actors than can act and are in the same sector
    for (int i = 0; i < pActors.size(); ++i) {
        if (pActors[i].CanAct() && pActors[i].sectorId == pBLVRenderParams->uPartySectorID) {
            auto found = std::find_if(pickedActorIds.begin(), pickedActorIds.end(), [&] (int id) { return id == i; });
            if (found == pickedActorIds.end()) {
                pActors[i].attributes |= ACTOR_ACTIVE;
                pickedActorIds.push_back(i);
            }
        }
    }

    // add any actors that are active and have previosuly detected the player
    for (const auto &[actorId, _] : activeActorsDistances) {
        if (pActors[actorId].attributes & (ACTOR_ACTIVE | ACTOR_NEARBY) && pActors[actorId].CanAct()) {
            auto found = std::find_if(pickedActorIds.begin(), pickedActorIds.end(), [&actorId = actorId] (int id) { return id == actorId; });
            if (found == pickedActorIds.end()) {
                pActors[actorId].attributes |= ACTOR_ACTIVE;
                pickedActorIds.push_back(actorId);
            }
        }
    }

    // activate ai state for first 30 actors from list
    for (int i = 0; (i < 30) && (i < pickedActorIds.size()); i++) {
        ai_near_actors_ids[i] = pickedActorIds[i];
        pActors[pickedActorIds[i]].attributes |= ACTOR_FULL_AI_STATE;
    }

    ai_arrays_size = std::min(30, (int)pickedActorIds.size());

    return ai_arrays_size;
}

//----- (004070EF) --------------------------------------------------------
bool Detect_Between_Objects(unsigned int uObjID, unsigned int uObj2ID) {
    // get object 1 info
    int obj1_pid = PID_ID(uObjID);
    int obj1_x, obj1_y, obj1_z, eyeheight;
    int obj1_sector;

    switch (PID_TYPE(uObjID)) {
        case OBJECT_Decoration:
            obj1_x = pLevelDecorations[obj1_pid].vPosition.x;
            obj1_y = pLevelDecorations[obj1_pid].vPosition.y;
            obj1_z = pLevelDecorations[obj1_pid].vPosition.z;
            obj1_sector = pIndoor->GetSector(obj1_x, obj1_y, obj1_z);
            break;
        case OBJECT_Actor:
            obj1_x = pActors[obj1_pid].pos.x;
            obj1_y = pActors[obj1_pid].pos.y;
            eyeheight = (float)pActors[obj1_pid].height * 0.69999999;
            obj1_z = eyeheight + pActors[obj1_pid].pos.z;
            obj1_sector = pActors[obj1_pid].sectorId;
            break;
        case OBJECT_Item:
            obj1_x = pSpriteObjects[obj1_pid].vPosition.x;
            obj1_y = pSpriteObjects[obj1_pid].vPosition.y;
            obj1_z = pSpriteObjects[obj1_pid].vPosition.z;
            obj1_sector = pSpriteObjects[obj1_pid].uSectorID;
            break;
        default:
            return 0;
    }

    // get object 2 info
    int obj2_pid = PID_ID(uObj2ID);
    int obj2_x, obj2_y, obj2_z, eyeheight2;
    int obj2_sector;

    switch (PID_TYPE(uObj2ID)) {
        case OBJECT_Decoration:
            obj2_z = pLevelDecorations[obj2_pid].vPosition.z;
            obj2_x = pLevelDecorations[obj2_pid].vPosition.x;
            obj2_y = pLevelDecorations[obj2_pid].vPosition.y;
            obj2_sector = pIndoor->GetSector(obj2_x, obj2_y, obj2_z);
            break;
        case OBJECT_Character:
            obj2_x = pParty->pos.x;
            obj2_z = pParty->eyeLevel + pParty->pos.z;
            obj2_y = pParty->pos.y;
            obj2_sector = pBLVRenderParams->uPartyEyeSectorID;
            break;
        case OBJECT_Actor:
            obj2_y = pActors[obj2_pid].pos.y;
            obj2_x = pActors[obj2_pid].pos.x;
            eyeheight2 = (float)pActors[obj2_pid].height * 0.69999999;
            obj2_z = eyeheight2 + pActors[obj2_pid].pos.z;
            obj2_sector = pActors[obj2_pid].sectorId;
            break;
        case OBJECT_Item:
            obj2_x = pSpriteObjects[obj2_pid].vPosition.x;
            obj2_z = pSpriteObjects[obj2_pid].vPosition.z;
            obj2_y = pSpriteObjects[obj2_pid].vPosition.y;
            obj2_sector = pSpriteObjects[obj2_pid].uSectorID;
            break;
        default:
            return 0;
    }

    // get distance between objects
    float dist_x = obj2_x - obj1_x;
    float dist_y = obj2_y - obj1_y;
    float dist_z = obj2_z - obj1_z;
    float dist_3d = sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
    // range check
    if (dist_3d > 5120) return 0;

    // if in range always detected outdoors
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) return 1;

    // monster in same sector with player/ monster
    if (obj1_sector == obj2_sector) return 1;

    // normalising
    float rayxnorm = dist_x / dist_3d;
    float rayynorm = dist_y / dist_3d;
    float rayznorm = dist_z / dist_3d;

    // extents for boundary checks
    int higher_z = std::max(obj1_z, obj2_z);
    int lower_z = std::min(obj1_z, obj2_z);
    int higher_y = std::max(obj1_y, obj2_y);
    int lower_y = std::min(obj1_y, obj2_y);
    int higher_x = std::max(obj1_x, obj2_x);
    int lower_x = std::min(obj1_x, obj2_x);

    // search starts for object
    int sectors_visited = 0;
    int current_sector = obj1_sector;
    int next_sector = 0;
    BLVFace *portalface;
    Vec3s *portalverts;

    // loop through portals
    for (int current_portal = 0; current_portal < pIndoor->pSectors[current_sector].uNumPortals; current_portal++) {
        portalface = &pIndoor->pFaces[pIndoor->pSectors[current_sector].pPortals[current_portal]];
        portalverts = &pIndoor->pVertices[*portalface->pVertexIDs];

        // fixpoint   ray ob1 to portal dot normal
        float obj1portaldot = portalface->facePlane.normal.z * (portalverts->z - obj1_z) +
                              portalface->facePlane.normal.y * (portalverts->y - obj1_y) +
                              portalface->facePlane.normal.x * (portalverts->x - obj1_x);

        // flip norm if we are not looking out from current sector
        if (current_sector != portalface->uSectorID) obj1portaldot = -obj1portaldot;

        // obj1 sees back of, but is not on the portal so skip
        if (obj1portaldot >= 0 && portalverts->x != obj1_x && portalverts->y != obj1_y && portalverts->z != obj1_z)
            continue;

        // bounds check
        if (lower_x > portalface->pBounding.x2 || higher_x < portalface->pBounding.x1 ||
            lower_y > portalface->pBounding.y2 || higher_y < portalface->pBounding.y1 ||
            lower_z > portalface->pBounding.z2 || higher_z < portalface->pBounding.z1) {
            continue;
        }

        // dot plane normal with obj ray
        float v32 = portalface->facePlane.normal.x * rayxnorm;
        float v34 = portalface->facePlane.normal.y * rayynorm;
        float v33 = portalface->facePlane.normal.z * rayznorm;

        // if face is parallel == 0 dont check LOS  -- add epsilon?
        float facenotparallel = v32 + v33 + v34;
        if (facenotparallel) {
            // point to plance distance
            float pointplanedist = -(portalface->facePlane.dist +
                  obj1_z * portalface->facePlane.normal.z +
                  obj1_x * portalface->facePlane.normal.x +
                  obj1_y * portalface->facePlane.normal.y);

            // epsilon check?
            if (abs(pointplanedist) / 16384.0 > abs(facenotparallel)) continue;

            // how far along line intersection is
            float intersect = pointplanedist / facenotparallel;

            // less than zero and intersection is behind target
            if (intersect < 0) continue;

            // check if point along ray is in portal face
            Vec3i pos = Vec3i(obj1_x + (rayxnorm * intersect) + 0.5,
                obj1_y + (rayynorm * intersect) + 0.5,
                obj1_z + (rayznorm * intersect) + 0.5);
            if (!portalface->Contains(pos, MODEL_INDOOR)) {
                // not visible through this portal
                continue;
            }

            // if there is no next sector turn back
            if (portalface->uSectorID == current_sector)
                next_sector = portalface->uBackSectorID;
            else
                next_sector = portalface->uSectorID;

            // no more portals, quit
            if (next_sector == current_sector) break;

            ++sectors_visited;
            current_sector = next_sector;

            // found object / player / monster, quit
            if (next_sector == obj2_sector) return 1;

            current_sector = next_sector;

            // did we hit limit for portals?
            // does the next room have portals?
            if (sectors_visited < 30 && pIndoor->pSectors[current_sector].uNumPortals > 0) {
                current_portal = -1;
                continue;
            } else {
                break;
            }
        }
    }
    // did we stop in the sector where object is?
    if (current_sector != obj2_sector) return 0;
    return 1;
}

//----- (00450B0A) --------------------------------------------------------
bool SpawnActor(unsigned int uMonsterID) {
    Actor *actor = AllocateActor(true);
    if (!actor)
        return false;

    unsigned int v1 = uMonsterID;
    if (uMonsterID >= pMonsterList->pMonsters.size())
        v1 = 0;

    Vec3i pOut;
    Vec3i::rotate(200, pParty->_viewYaw, 0, pParty->pos, &pOut.x, &pOut.y, &pOut.z);

    actor->name = pMonsterStats->pInfos[v1 + 1].pName;
    actor->currentHP = pMonsterStats->pInfos[v1 + 1].uHP;
    actor->monsterInfo = pMonsterStats->pInfos[v1 + 1];
    actor->word_000086_some_monster_id = v1 + 1;
    actor->radius = pMonsterList->pMonsters[v1].uMonsterRadius;
    actor->height = pMonsterList->pMonsters[v1].uMonsterHeight;
    actor->moveSpeed = pMonsterList->pMonsters[v1].uMovementSpeed;
    actor->initialPosition.x = pOut.x;
    actor->pos.x = pOut.x;
    actor->tetherDistance = 256;
    actor->initialPosition.y = pOut.y;
    actor->pos.y = pOut.y;
    actor->initialPosition.z = pOut.z;
    actor->pos.z = pOut.z;

    pSprites_LOD->DeleteSomeSprites();

    actor->PrepareSprites(1);

    return true;
}

//----- (0044FA4C) --------------------------------------------------------
void Spawn_Light_Elemental(int spell_power, CharacterSkillMastery caster_skill_mastery, int duration_game_seconds) {
    // size_t uActorIndex;            // [sp+10h] [bp-10h]@6

    const char *cMonsterName;       // [sp-4h] [bp-24h]@2
    if (caster_skill_mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER)
        cMonsterName = "Elemental Light C";
    else if (caster_skill_mastery == CHARACTER_SKILL_MASTERY_MASTER)
        cMonsterName = "Elemental Light B";
    else
        cMonsterName = "Elemental Light A";
    unsigned int uMonsterID = pMonsterList->GetMonsterIDByName(cMonsterName);

    Actor *actor = AllocateActor(false);
    if (!actor)
        return; // Too many actors.

    int partySectorId = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        partySectorId = pBLVRenderParams->uPartySectorID;

    int radius = uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ? 128 : 64;
    int angle = grng->random(2048);

    actor->name = pMonsterStats->pInfos[uMonsterID + 1].pName;
    actor->currentHP = pMonsterStats->pInfos[uMonsterID + 1].uHP;
    actor->monsterInfo = pMonsterStats->pInfos[uMonsterID + 1];
    actor->word_000086_some_monster_id = uMonsterID + 1;
    actor->radius = pMonsterList->pMonsters[uMonsterID].uMonsterRadius;
    actor->height = pMonsterList->pMonsters[uMonsterID].uMonsterHeight;
    actor->monsterInfo.uTreasureDiceRolls = 0;
    actor->monsterInfo.uTreasureType = 0;
    actor->monsterInfo.uExp = 0;
    actor->moveSpeed = pMonsterList->pMonsters[uMonsterID].uMovementSpeed;
    actor->initialPosition.x = pParty->pos.x + TrigLUT.cos(angle) * radius;
    actor->initialPosition.y = pParty->pos.y + TrigLUT.sin(angle) * radius;
    actor->initialPosition.z = pParty->pos.z;
    actor->pos = actor->initialPosition;
    actor->tetherDistance = 256;
    actor->sectorId = partySectorId;
    actor->PrepareSprites(0);
    actor->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
    actor->ally = 9999;
    actor->group = 0;
    actor->currentActionTime = 0;
    actor->aiState = Summoned;
    actor->currentActionLength = 256;
    actor->UpdateAnimation();

    int sectorId = pIndoor->GetSector(actor->pos);
    int zlevel;
    int zdiff;
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ||
            sectorId == partySectorId &&
            (zlevel = BLV_GetFloorLevel(actor->pos, sectorId), zlevel != -30000) &&
            (zdiff = abs(zlevel - pParty->pos.z), zdiff <= 1024)) {
        actor->summonerId = PID(OBJECT_Character, spell_power);

        GameTime spell_length = GameTime::FromSeconds(duration_game_seconds);

        actor->buffs[ACTOR_BUFF_SUMMONED].Apply((pParty->GetPlayingTime() + spell_length),
                                                caster_skill_mastery, spell_power, 0, 0);
    } else {
        actor->Remove();
    }
}

//----- (0044F57C) --------------------------------------------------------
void SpawnEncounter(MapInfo *pMapInfo, SpawnPoint *spawn, int a3, int a4, int a5) {
    // a3 for abc modify
    // a4 count
    Assert(spawn->uKind == OBJECT_Actor);

    char v8;               // zf@5
    int v12;               // edx@9
    // int v18;               // esi@31
    int v23;               // edx@36
    int v24;        // edi@36
    int v25;               // ecx@36
    MonsterDesc *v27;      // edi@48
    signed int v28;        // eax@48
    int v32;               // eax@50
    int v37;               // eax@51
    int v38;               // eax@52
    int v39;               // edi@52
    std::string v40;       // [sp-18h] [bp-100h]@60
    // const char *v44;       // [sp-8h] [bp-F0h]@13
    std::string pTexture;        // [sp-4h] [bp-ECh]@9
                           //  char Str[32]; // [sp+Ch] [bp-DCh]@60
    std::string Str2;           // [sp+2Ch] [bp-BCh]@29
    MonsterInfo *Src;      // [sp+A8h] [bp-40h]@50
    int v50;               // [sp+ACh] [bp-3Ch]@47
    std::string Source;         // [sp+B0h] [bp-38h]@20
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
    int v7 = GetAlertStatus();

    if (v7)
        v8 = (spawn->uAttributes & 1) == 0;
    else
        v8 = (spawn->uAttributes & 1) == 1;
    if (v8) return;

    // result = (void *)(spawn->uIndex - 1);
    NumToSpawn = 1;
    switch (spawn->uMonsterIndex - 1) {
        case 0:
            // v9 = pMapInfo->uEncounterMonster1AtLeast;
            // v10 = rand();
            // v11 = pMapInfo->uEncounterMonster1AtMost;
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            v12 = grng->random(pMapInfo->uEncounterMonster1AtMost - pMapInfo->uEncounterMonster1AtLeast + 1);
            // v13 = pMapInfo->Dif_M1;
            v57 = pMapInfo->Dif_M1;
            NumToSpawn = pMapInfo->uEncounterMonster1AtLeast + v12;
            Source = pMapInfo->pEncounterMonster1Texture;
            break;
        case 3:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s A";
            Source = pMapInfo->pEncounterMonster1Texture + " A";
            break;
        case 4:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s A";
            Source = pMapInfo->pEncounterMonster2Texture + " A";
            break;
        case 5:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s A";
            Source = pMapInfo->pEncounterMonster3Texture + " A";
            break;
        case 1:
            // v9 = pMapInfo->uEncounterMonster2AtLeast;
            // v14 = rand();
            // v15 = pMapInfo->uEncounterMonster2AtMost;
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            v12 = grng->random(pMapInfo->uEncounterMonster2AtMost - pMapInfo->uEncounterMonster2AtLeast + 1);
            // v13 = pMapInfo->Dif_M2;
            v57 = pMapInfo->Dif_M2;
            NumToSpawn = pMapInfo->uEncounterMonster2AtLeast + v12;
            Source = pMapInfo->pEncounterMonster2Texture;
            break;
        case 6:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s B";
            Source = pMapInfo->pEncounterMonster1Texture + " B";
            break;
        case 7:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s B";
            Source = pMapInfo->pEncounterMonster2Texture + " B";
            break;
        case 8:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s B";
            Source = pMapInfo->pEncounterMonster3Texture + " B";
            break;
        case 2:
            // v9 = pMapInfo->uEncounterMonster3AtLeast;
            // v16 = rand();
            // v17 = pMapInfo->uEncounterMonster3AtMost;
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            v12 = grng->random(pMapInfo->uEncounterMonster3AtMost - pMapInfo->uEncounterMonster3AtLeast + 1);
            // v13 = pMapInfo->Dif_M3;
            v57 = pMapInfo->Dif_M3;
            NumToSpawn = pMapInfo->uEncounterMonster3AtLeast + v12;
            Source = pMapInfo->pEncounterMonster3Texture;
            break;
        case 9:
            // pTexture = pMapInfo->pEncounterMonster1Texture;
            // v44 = "%s C";
            Source = pMapInfo->pEncounterMonster1Texture + " C";
            break;
        case 10:
            // pTexture = pMapInfo->pEncounterMonster2Texture;
            // v44 = "%s C";
            Source = pMapInfo->pEncounterMonster2Texture + " C";
            break;
        case 11:
            // pTexture = pMapInfo->pEncounterMonster3Texture;
            // v44 = "%s C";
            Source = pMapInfo->pEncounterMonster3Texture + " C";
            break;
        default:
            return;
    }

    if (Source[0] == '0') return;

    v57 += a3;

    // if (v57 == 4) __debugbreak();
    if (v57 > 3) v57 = 3;

    Str2 = Source;
    if (a4) NumToSpawn = a4;
    // v18 = NumToSpawn;
    if (NumToSpawn <= 0) return;

    pSector = 0;
    pPosX = spawn->vPosition.x;
    a4 = spawn->vPosition.y;
    a3 = spawn->vPosition.z;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        pSector = pIndoor->GetSector(spawn->vPosition);
    v53 = 0;
    v52 = (((uCurrentlyLoadedLevelType != LEVEL_OUTDOOR) - 1) & 0x40) + 64;



    // if (v57 == 4) return;

    // spawning loop
    for (int i = v53; i < NumToSpawn; ++i) {
        Actor *pMonster = AllocateActor(true);
        if (!pMonster)
            continue;

        // random monster levels ABC
        if (v57) {
            // if (v57 > 3) v57 = 3;

            v23 = grng->random(100);
            v24 = 3;  // 2 , 10 , 20 - C
            v25 = (uint16_t)word_4E8152[3 * v57];  // v57 should be 1,2,3
            if (v23 >= v25) {
                if (v23 < v25 + (uint16_t)word_4E8152[3 * v57 + 1]) {
                    v24 = 2;  // 8 , 20 , 30 - B
                }
            } else {
                v24 = 1;  // 90 , 70 , 50 - A
            }

            if (v24 == 1) {
                Str2 = Source + " A";
            } else if (v24 == 2) {
                Str2 = Source + " B";
            } else {
                if (v24 != 3) continue;
                Str2 = Source + " C";
            }
        }

        v50 = pMonsterList->GetMonsterIDByName(Str2);
        pTexture = Str2;
        if ((int16_t)v50 == -1) {
            logger->warning("Can't create random monster: '{}'! See MapStats.txt and Monsters.txt!", pTexture);
            Engine_DeinitializeAndTerminate(0);
        }

        v27 = &pMonsterList->pMonsters[(int16_t)v50];
        v28 = pMonsterStats->FindMonsterByTextureName(pTexture);
        if (!v28) v28 = 1;
        Src = &pMonsterStats->pInfos[v28];
        pMonster->name = Src->pName;
        pMonster->currentHP = Src->uHP;

        // memcpy(&pMonster->pMonsterInfo, Src, sizeof(MonsterInfo));  // Uninitialized portail memory access

        pMonster->monsterInfo = pMonsterStats->pInfos[v28];

        pMonster->word_000086_some_monster_id = v50 + 1;
        pMonster->radius = v27->uMonsterRadius;
        pMonster->height = v27->uMonsterHeight;
        pMonster->moveSpeed = v27->uMovementSpeed;
        pMonster->initialPosition.x = spawn->vPosition.x;
        pMonster->pos.x = spawn->vPosition.x;
        pMonster->tetherDistance = 256;
        pMonster->initialPosition.y = a4;
        pMonster->pos.y = a4;
        pMonster->initialPosition.z = a3;
        pMonster->pos.z = a3;
        pMonster->sectorId = pSector;
        pMonster->group = spawn->uGroup;
        pMonster->PrepareSprites(0);
        pMonster->monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
        v32 = grng->random(2048);
        a3 = TrigLUT.cos(v32) * v52;
        pPosX = a3 + spawn->vPosition.x;
        a3 = TrigLUT.sin(v32) * v52;
        a4 = a3 + spawn->vPosition.y;
        a3 = spawn->vPosition.z;
        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            if (a5)
                pMonster->attributes |= ACTOR_AGGRESSOR;
            continue;
        }
        v37 = pIndoor->GetSector(pPosX, a4, spawn->vPosition.z);
        if (v37 == pSector) {
            v38 = BLV_GetFloorLevel(Vec3i(pPosX, a4, a3), v37);
            v39 = v38;
            if (v38 != -30000) {
                if (abs(v38 - a3) <= 1024) {
                    a3 = v39;
                    if (a5)
                        pMonster->attributes |= ACTOR_AGGRESSOR;
                    continue;
                }
            }
        }

        // Actor was spawned too far away, remove it.
        pMonster->Remove();

        // v53 = (char *)v53 + 1;
        // result = v53;
    }
    // while ( (signed int)v53 < NumToSpawn );
}

void evaluateAoeDamage() {
    SpriteObject *pSpriteObj = nullptr;

    for (AttackDescription &attack : attackList) {
        ObjectType attackerType = PID_TYPE(attack.pid);
        int attackerId = PID_ID(attack.pid);
        Vec3i attackVector = Vec3i(0, 0, 0);

        // attacker is an item (sprite)
        if (attackerType == OBJECT_Item) {
            pSpriteObj = &pSpriteObjects[attackerId];
            attackerType = PID_TYPE(pSpriteObjects[attackerId].spell_caster_pid);
            attackerId = PID_ID(pSpriteObjects[attackerId].spell_caster_pid);
        }

        if (attack.isMelee) {
            unsigned int targetId = PID_ID(ai_near_actors_targets_pid[attackerId]);
            ObjectType targetType = PID_TYPE(ai_near_actors_targets_pid[attackerId]);
            Actor *actor = &pActors[targetId];

            if (targetType != OBJECT_Actor) {
                if (targetType == OBJECT_Character) {  // party damage from monsters
                    int distanceSq = (pParty->pos + Vec3i(0, 0, pParty->height / 2) - attack.pos).lengthSqr();
                    int attackRangeSq = (attack.attackRange + 32) * (attack.attackRange + 32);

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(pParty->pos + Vec3i(0, 0, pParty->eyeLevel), attack.pos)) {
                            DamageCharacterFromMonster(attack.pid, attack.attackSpecial, &attackVector, stru_50C198.which_player_to_attack(&pActors[attackerId]));
                        }
                    }
                }
            } else {  // Actor (peasant) damage from monsters
                if (actor->buffs[ACTOR_BUFF_PARALYZED].Active() || actor->CanAct()) {
                    Vec3i distanceVec = actor->pos + Vec3i(0, 0, actor->height / 2) - attack.pos;
                    int distanceSq = distanceVec.lengthSqr();
                    int attackRange = attack.attackRange + actor->radius;
                    int attackRangeSq = attackRange * attackRange;
                    attackVector = Vec3i(distanceVec.x, distanceVec.y, actor->pos.z);

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(actor->pos + Vec3i(0, 0, 50), attack.pos)) {
                            normalize_to_fixpoint(&attackVector.x, &attackVector.y, &attackVector.z);
                            Actor::ActorDamageFromMonster(attack.pid, targetId, &attackVector, attack.attackSpecial);
                        }
                    }
                }
            }
        } else {  // damage from AOE spells
            int distanceSq = (pParty->pos + Vec3i(0, 0, pParty->height / 2) - attack.pos).lengthSqr();
            int attackRangeSq = (attack.attackRange + 32) * (attack.attackRange + 32);

            // check spell in range of party
            if (distanceSq < attackRangeSq) {  // party damage
                // check line of sight to party
                if (Check_LineOfSight(pParty->pos + Vec3i(0, 0, pParty->eyeLevel), attack.pos)) {
                    for (int i = 0; i < pParty->pCharacters.size(); i++) {
                        if (pParty->pCharacters[i].conditions.HasNone({CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
                            DamageCharacterFromMonster(attack.pid, attack.attackSpecial, &attackVector, i);
                        }
                    }
                }
            }

            for (int actorID = 0; actorID < pActors.size(); ++actorID) {
                if (pActors[actorID].CanAct()) {
                    Vec3i distanceVec = pActors[actorID].pos + Vec3i(0, 0, pActors[actorID].height / 2) - attack.pos;
                    int distanceSq = distanceVec.lengthSqr();
                    int attackRange = attack.attackRange + pActors[actorID].radius;
                    int attackRangeSq = attackRange * attackRange;
                    // TODO: using absolute Z here is BS, it's used as speed in ItemDamageFromActor
                    attackVector = Vec3i(distanceVec.x, distanceVec.y, pActors[actorID].pos.z);

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(pActors[actorID].pos + Vec3i(0, 0, 50), attack.pos)) {
                            normalize_to_fixpoint(&attackVector.x, &attackVector.y, &attackVector.z);
                            switch (attackerType) {
                                case OBJECT_Character:
                                    Actor::DamageMonsterFromParty(attack.pid, actorID, &attackVector);
                                    break;
                                case OBJECT_Actor:
                                    if (pSpriteObj && pActors[attackerId].GetActorsRelation(&pActors[actorID])) {
                                        Actor::ActorDamageFromMonster(attack.pid, actorID, &attackVector, pSpriteObj->spellCasterAbility);
                                    }
                                    break;
                                case OBJECT_Item:
                                    ItemDamageFromActor(attack.pid, actorID, &attackVector);
                                    break;
                                default:
                                    assert(false);
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    attackList.clear();
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
                         Vec3i *pVelocity) {
    int v6;      // eax@4
    int damage;  // edi@4
    int a2a;     // [sp+Ch] [bp-4h]@8

    if (!pActors[uActorID].IsNotAlive()) {
        if (PID_TYPE(uObjID) == OBJECT_Item) {
            if (pSpriteObjects[PID_ID(uObjID)].uSpellID) {
                v6 = CalcSpellDamage(
                    pSpriteObjects[PID_ID(uObjID)].uSpellID,
                    pSpriteObjects[PID_ID(uObjID)].spell_level,
                    pSpriteObjects[PID_ID(uObjID)].spell_skill,
                    pActors[uActorID].currentHP);
                damage = pActors[uActorID].CalcMagicalDamageToActor(
                    DMGT_FIRE, v6);
                pActors[uActorID].currentHP -= damage;
                if (damage) {
                    if (pActors[uActorID].currentHP > 0)
                        Actor::AI_Stun(uActorID, uObjID, 0);
                    else
                        Actor::Die(uActorID);
                    a2a = 20 * damage /
                          (signed int)pActors[uActorID].monsterInfo.uHP;
                    if (20 * damage /
                            (signed int)pActors[uActorID].monsterInfo.uHP >
                        10)
                        a2a = 10;
                    if (!MonsterStats::BelongsToSupertype(
                            pActors[uActorID].monsterInfo.uID,
                            MONSTER_SUPERTYPE_TREANT)) {
                        pVelocity->x = fixpoint_mul(a2a, pVelocity->x);
                        pVelocity->y = fixpoint_mul(a2a, pVelocity->y);
                        pVelocity->z = fixpoint_mul(a2a, pVelocity->z);
                        pActors[uActorID].speed.x =
                            50 * (short)pVelocity->x;
                        pActors[uActorID].speed.y =
                            50 * (short)pVelocity->y;
                        pActors[uActorID].speed.z =
                            50 * (short)pVelocity->z;
                    }
                    Actor::AddOnDamageOverlay(uActorID, 1, damage);
                } else {
                    Actor::AI_Stun(uActorID, uObjID, 0);
                }
            }
        }
    }
}

Actor *AllocateActor(bool appendOnly) {
    if (!appendOnly) {
        for (size_t i = 0; i < pActors.size(); i++) {
            if (pActors[i].aiState == Removed) {
                pActors[i].Reset();
                return &pActors[i];
            }
        }
    }

    if (pActors.size() >= 500)
        return nullptr; // Too many actors.

    return &pActors.emplace_back(Actor(pActors.size()));
}
