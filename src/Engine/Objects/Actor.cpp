#include "Engine/Objects/Actor.h"

#include <algorithm>
#include <deque>
#include <string>
#include <utility>
#include <vector>
#include <optional>

#include "Engine/Engine.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Localization.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/Spells.h"
#include "Engine/AttackList.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"

// should be injected into Actor but struct size cant be changed
static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

// Using deque for pointer stability
std::deque<Actor> pActors;

stru319 stru_50C198;  // idb

static constexpr IndexedArray<int, HOSTILITY_FIRST, HOSTILITY_LAST> _4DF380_hostilityRanges = {
    {HOSTILITY_FRIENDLY, 0},
    {HOSTILITY_CLOSE, 1024},
    {HOSTILITY_SHORT, 2560},
    {HOSTILITY_MEDIUM, 5120},
    {HOSTILITY_LONG, 10240}
};

std::array<int16_t, 11> word_4E8152 = {{0, 0, 0, 90, 8, 2, 70, 20, 10, 50, 30}};  // level spawn monster levels ABC

//----- (0042FB5C) --------------------------------------------------------
// True if monster should play attack animation when casting this spell.
bool ShouldMonsterPlayAttackAnim(SpellId spell_id) {
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
void Actor::DrawHealthBar(Actor *actor, Recti window) {
    // bar length
    unsigned int bar_length;
    if (actor->monsterInfo.hp <= 25)
        bar_length = 25;
    else if (actor->monsterInfo.hp < 200)
        bar_length = actor->monsterInfo.hp;
    else
        bar_length = 200;

    // bar colour
    GraphicsImage *bar_image = game_ui_monster_hp_green;
    if (actor->hp <= (0.34 * actor->monsterInfo.hp))
        bar_image = game_ui_monster_hp_red;
    else if (actor->hp <= (0.67 * actor->monsterInfo.hp))
        bar_image = game_ui_monster_hp_yellow;

    // how much of bar is filled
    unsigned int bar_filled_length = bar_length;
    if (actor->hp < (int)actor->monsterInfo.hp)
        bar_filled_length = (bar_length * actor->hp) / actor->monsterInfo.hp;

    // centralise for clipping and draw
    unsigned int uX = window.x + (signed int)(window.w - bar_length) / 2;

    render->SetUIClipRect(Recti(uX, window.y + 32, bar_length, 20));
    render->DrawQuad2D(game_ui_monster_hp_background, {static_cast<int>(uX), window.y + 32});

    render->SetUIClipRect(Recti(uX, window.y + 32, bar_filled_length, 20));
    render->DrawQuad2D(bar_image, {static_cast<int>(uX), window.y + 34});

    // draw hp bar ends
    render->ResetUIClipRect();
    render->DrawQuad2D(game_ui_monster_hp_border_left, {static_cast<int>(uX) - 5, window.y + 32});
    render->DrawQuad2D(game_ui_monster_hp_border_right, {static_cast<int>(uX + bar_length), window.y + 32});
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

void npcSetItem(int npc, ItemId item, int a3) {
    for (unsigned i = 0; i < pActors.size(); i++) {
        if (pActors[i].npcId == npc) {
            Actor::giveItem(i, item, a3);
        }
    }
}

void Actor::giveItem(signed int uActorID, ItemId uItemID, unsigned int bGive) {
    if ((uActorID >= 0) && (signed int)uActorID <= (signed int)(pActors.size() - 1)) {
        if (bGive) {
            if (pActors[uActorID].carriedItemId == ITEM_NULL)
                pActors[uActorID].carriedItemId = uItemID;
            else if (pActors[uActorID].items[0].itemId == ITEM_NULL)
                pActors[uActorID].items[0].itemId = uItemID;
            else if (pActors[uActorID].items[1].itemId == ITEM_NULL)
                pActors[uActorID].items[1].itemId = uItemID;
        } else {
            if (pActors[uActorID].carriedItemId == uItemID)
                pActors[uActorID].carriedItemId = ITEM_NULL;
            else if (pActors[uActorID].items[0].itemId == uItemID)
                pActors[uActorID].items[0].Reset();
            else if (pActors[uActorID].items[1].itemId == uItemID)
                pActors[uActorID].items[1].Reset();
        }
    }
}

//----- (0040894B) --------------------------------------------------------
bool Actor::CanAct() const {
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
    if (this->items[3].itemId == ITEM_NULL) {
        if (this->monsterInfo.goldDiceRolls) {
            v2 = grng->randomDice(this->monsterInfo.goldDiceRolls, this->monsterInfo.goldDiceSides);
            if (v2) {
                this->items[3].itemId = ITEM_GOLD_SMALL;
                this->items[3].goldAmount = v2;
            }
        }
    }
    if (grng->random(100) < this->monsterInfo.treasureDropChance) {
        if (this->monsterInfo.treasureLevel != ITEM_TREASURE_LEVEL_INVALID)
            pItemTable->generateItem(this->monsterInfo.treasureLevel,
                                     this->monsterInfo.treasureType,
                                     &this->items[2]);
    }
    this->attributes |= ACTOR_HAS_ITEM;
}

//----- (00404AC7) --------------------------------------------------------
void Actor::AI_SpellAttack(unsigned int uActorID, AIDirection *pDir,
                           SpellId uSpellID, ActorAbility a4, CombinedSkillValue uSkillMastery) {
    Duration spellLength;

    SpriteObject sprite;
    Actor *actorPtr = &pActors[uActorID];
    int realPoints = uSkillMastery.level();
    Mastery masteryLevel = uSkillMastery.mastery();
    int distancemod = 3;
    int spriteId = -1;

    static const int ONE_THIRD_PI = TrigLUT.uIntegerPi / 3;

    if (pDir->uDistance < meleeRange) {
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
            sprite.spell_skill = MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3f(0, 0, actorPtr->height / 2);
            sprite.uFacing = (short)pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.timeSinceCreated = 0_ticks;
            sprite.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
            sprite.spell_target_pid = Pid();
            sprite.field_60_distance_related_prolly_lod = distancemod;
            sprite.spellCasterAbility = ABILITY_SPELL1;

            spriteId = sprite.Create(pDir->uYawAngle, pDir->uPitchAngle, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Sprite, spriteId));
            }
            break;

        case SPELL_FIRE_HASTE:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromMinutes(40 + 2 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromMinutes(45 + 3 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HASTE].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.OrangeyRed);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_FIRE_METEOR_SHOWER:
        {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                break;
            }

            int meteors;
            switch (masteryLevel) {
                default:
                    assert(false);
                case MASTERY_NOVICE:
                    meteors = 8;
                    break;
                case MASTERY_EXPERT:
                    meteors = 10;
                    break;
                case MASTERY_MASTER:
                    meteors = 12;
                    break;
                case MASTERY_GRANDMASTER:
                    meteors = 14;
                    break;
            }

            int j = 0, k = 0;
            int yaw, pitch;
            for (int i = 0; i < meteors; i++) {
                int originHeight = grng->random(1000);
                if (Vec3f(j, k, originHeight - 2500).length() <= 1.0f) {
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
                sprite.spell_skill = MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
                sprite.vPosition = pParty->pos + Vec3f(0, 0, originHeight + 2500);
                sprite.uSpellID = SPELL_FIRE_METEOR_SHOWER;
                sprite.uAttributes = 0;
                sprite.uSectorID = 0;
                sprite.timeSinceCreated = 0_ticks;
                sprite.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
                sprite.spell_target_pid = Pid();
                sprite.uFacing = yaw;
                sprite.uSoundID = 0;
                sprite.field_60_distance_related_prolly_lod = distancemod;
                sprite.spellCasterAbility = ABILITY_SPELL1;

                spriteId = sprite.Create(yaw, pitch, pObjectList->pObjects[sprite.uObjectDescID].uSpeed, 0);
                j = grng->random(1024) - 512;
                k = grng->random(1024) - 512;
            }
            if (spriteId != -1) {
                pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Sprite, spriteId));
            }
            break;
        }

        case SPELL_AIR_SPARKS:
        {
            int sparks;
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                    sparks = 3;
                    break;
                case MASTERY_EXPERT:
                    sparks = 5;
                    break;
                case MASTERY_MASTER:
                    sparks = 7;
                    break;
                case MASTERY_GRANDMASTER:
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
            sprite.spell_skill = MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3f(0, 0, actorPtr->height / 2);
            sprite.uFacing = pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
            sprite.timeSinceCreated = 0_ticks;
            sprite.spell_target_pid = Pid();
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
                pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Sprite, spriteId));
            }
            break;
        }

        case SPELL_AIR_SHIELD:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 5 * realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 15 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromHours(64 + realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_SHIELD].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            // Spell sound was missing from before
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_EARTH_STONESKIN:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 5 * realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 15 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromHours(64 + realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_STONESKIN].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.Cioccolato);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_SPIRIT_BLESS:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 5 * realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 15 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_BLESS].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_SPIRIT_FATE:
        {
            int spellPower;
            switch (masteryLevel) {
                default:
                    assert(false);
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellPower = 2 * realPoints + 40;
                    break;
                case MASTERY_MASTER:
                    spellPower = 3 * realPoints + 60;
                    break;
                case MASTERY_GRANDMASTER:
                    spellPower = 6 * realPoints + 120;
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_FATE].Apply(pParty->GetPlayingTime() + Duration::fromMinutes(5), masteryLevel, spellPower, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;
        }

        case SPELL_SPIRIT_HEROISM:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 5 * realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 15 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromHours(1) + Duration::fromMinutes(4 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HEROISM].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.RioGrande);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_BODY_HAMMERHANDS:
            // TODO(Nik-RE-dev): calculation of duration is strange
            actorPtr->buffs[ACTOR_BUFF_HAMMERHANDS]
                .Apply(pParty->GetPlayingTime() + Duration::fromHours(realPoints), masteryLevel, realPoints, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.JazzberryJam);
            pAudioPlayer->playSound(SOUND_51heroism03, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_BODY_POWER_CURE:
            actorPtr->hp += 5 * realPoints + 10;
            if (actorPtr->hp >= actorPtr->monsterInfo.hp) {
                actorPtr->hp = actorPtr->monsterInfo.hp;
            }
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.JazzberryJam);
            pAudioPlayer->playSound(SOUND_Fate, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_LIGHT_PARALYZE:
            // TODO(pskelton): This is a vanilla bug - monsters with instant targeting spells can't actually use them - #1246
            logger->info("Spell Paralyze cast - replaced with dispel");
            [[fallthrough]];
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
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_LIGHT_DAY_OF_PROTECTION:
        {
            // TODO(Nik-RE-dev): calculation of duration is strange
            int spellPower;
            switch (masteryLevel) {
                default:
                    assert(false);
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromMinutes(64 + 5 * realPoints);
                    spellPower = realPoints;
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromMinutes(64 + 15 * realPoints);
                    spellPower = 3 * realPoints;
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromMinutes(64 + 20 * realPoints);
                    spellPower = 4 * realPoints;
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_DAY_OF_PROTECTION].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, spellPower, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.White);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;
        }

        case SPELL_LIGHT_HOUR_OF_POWER:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                    spellLength = Duration::fromMinutes(64 + 5 * realPoints);
                    break;
                case MASTERY_MASTER:
                    spellLength = Duration::fromMinutes(64 + 15 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromMinutes(64 + 20 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }

            actorPtr->buffs[ACTOR_BUFF_HOUR_OF_POWER].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, realPoints + 5, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.White);
            pAudioPlayer->playSound(SOUND_9armageddon01, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        case SPELL_DARK_SHARPMETAL:
        {
            int pieces;
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                    pieces = 3;
                    break;
                case MASTERY_EXPERT:
                    pieces = 5;
                    break;
                case MASTERY_MASTER:
                    pieces = 7;
                    break;
                case MASTERY_GRANDMASTER:
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
            sprite.spell_skill = MASTERY_NONE; // TODO(captainurist): why do we ignore passed skill mastery?
            sprite.vPosition = actorPtr->pos + Vec3f(0, 0, actorPtr->height / 2);
            sprite.uFacing = pDir->uYawAngle;
            sprite.uSoundID = 0;
            sprite.uAttributes = 0;
            sprite.uSectorID = pIndoor->GetSector(sprite.vPosition);
            sprite.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
            sprite.timeSinceCreated = 0_ticks;
            sprite.spell_target_pid = Pid();
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
                pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Sprite, spriteId));
            }
            break;
        }

        case SPELL_DARK_PAIN_REFLECTION:
            // TODO(Nik-RE-dev): calculation of duration is strange
            switch (masteryLevel) {
                case MASTERY_NOVICE:
                case MASTERY_EXPERT:
                case MASTERY_MASTER:
                    spellLength = Duration::fromMinutes(64) + Duration::fromSeconds(5 * 30 * realPoints);
                    break;
                case MASTERY_GRANDMASTER:
                    spellLength = Duration::fromMinutes(64) + Duration::fromSeconds(15 * 30 * realPoints);
                    break;
                default:
                    assert(false);
                    break;
            }
            actorPtr->buffs[ACTOR_BUFF_PAIN_REFLECTION].Apply(pParty->GetPlayingTime() + spellLength, masteryLevel, 0, 0, 0);
            spell_fx_renderer->sparklesOnActorAfterItCastsBuff(actorPtr, colorTable.MediumGrey);
            pAudioPlayer->playSound(SOUND_Sacrifice2, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
            break;

        default:
            assert(false);
            break;
    }
}

unsigned short Actor::GetObjDescId(SpellId spellId) {
    return pObjectList->ObjectIDByItemID(SpellSpriteMapping[spellId]);  // crash here
}

bool Actor::ArePeasantsOfSameFaction(Actor *a1, Actor *a2) {
    MonsterType v2 = a1->ally;
    if (a1->ally == MONSTER_TYPE_INVALID)
        v2 = monsterTypeForMonsterId(a1->monsterInfo.id);

    MonsterType v3 = a2->ally;
    if (a2->ally == MONSTER_TYPE_INVALID)
        v3 = monsterTypeForMonsterId(a2->monsterInfo.id);

    if (isPeasant(v2) && isPeasant(v3) && raceForMonsterType(v2) == raceForMonsterType(v3) || v2 == v3)
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

    for (unsigned i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];
        if (!actor->CanAct() || i == uActorID) continue;

        if (Actor::ArePeasantsOfSameFaction(victim, actor)) {
            v4 = std::abs(actor->pos.x - victim->pos.x);
            v5 = std::abs(actor->pos.y - victim->pos.y);
            v6 = std::abs(actor->pos.z - victim->pos.z);
            if (int_get_vector_length(v4, v5, v6) < 4096) {
                actor->monsterInfo.hostilityType =
                    HOSTILITY_LONG;
                if (a2 == 1) actor->attributes |= ACTOR_AGGRESSOR;
            }
        }
    }
}

//----- (00404874) --------------------------------------------------------
void Actor::AI_RangedAttack(unsigned int uActorID, AIDirection *pDir,
                            MonsterProjectile type, ActorAbility a4) {
    if (type == MONSTER_PROJECTILE_NONE)
        return;

    char specAb;  // al@1
    int v13;      // edx@28

    SpriteObject a1;  // [sp+Ch] [bp-74h]@1
    a1.uType = spriteForMonsterProjectile(type);

    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    if (a1.uObjectDescID == 0) {
        logger->error("Item not found");
        return;
    }
    a1.containing_item.Reset();
    a1.uSpellID = SPELL_NONE;
    a1.vPosition.x = pActors[uActorID].pos.x;
    a1.vPosition.y = pActors[uActorID].pos.y;
    a1.vPosition.z = pActors[uActorID].pos.z + (pActors[uActorID].height * 0.75);
    a1.spell_level = 0;
    a1.spell_skill = MASTERY_NONE;
    a1.uFacing = pDir->uYawAngle;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a1.vPosition);
    a1.timeSinceCreated = 0_ticks;
    a1.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
    a1.spell_target_pid = Pid();
    if (pDir->uDistance < meleeRange)
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

    if (pActors[uActorID].monsterInfo.specialAbilityType == MONSTER_SPECIAL_ABILITY_MULTI_SHOT) {
        specAb = pActors[uActorID].monsterInfo.specialAbilityDamageDiceBonus;
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
    a1.spell_skill = MASTERY_NONE;
    a1.vPosition.x = pActors[uActorID].pos.x;
    a1.vPosition.y = pActors[uActorID].pos.y;
    a1.vPosition.z = pActors[uActorID].pos.z + (pActors[uActorID].height * 0.75);
    a1.uFacing = 0;
    a1.uSoundID = 0;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a1.vPosition);
    a1.timeSinceCreated = 0_ticks;
    a1.spell_caster_pid = Pid(OBJECT_Actor, uActorID);
    a1.spell_target_pid = Pid();
    a1.field_60_distance_related_prolly_lod = 3;
    a1.spellCasterAbility = ABILITY_SPECIAL;
    a1.Create(0, 0, 0, 0);
    return;
}

void Actor::GetDirectionInfo(Vec3f p1, Vec3f p2, AIDirection* pOut) {
    // TODO(pskelton): Should this be moved somewhere else - not actor related
    Vec3f dir = p2 - p1;
    float length = dir.length();
    float deltaX = p2.x - p1.x;
    float deltaY = p2.y - p1.y;
    float deltaZ = p2.z - p1.z;
    if (length <= 1.0) {
        pOut->vDirection = Vec3f(1, 0, 0);
        pOut->uDistance = 1;
        pOut->uDistanceXZ = 1;
        pOut->uYawAngle = 0;
        pOut->uPitchAngle = 0;
    } else {
        dir.normalize();
        pOut->vDirection = dir;
        pOut->uDistance = length;
        pOut->uDistanceXZ = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        pOut->uYawAngle = TrigLUT.atan2(deltaX, deltaY);
        pOut->uPitchAngle = TrigLUT.atan2(pOut->uDistanceXZ, deltaZ);
    }
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
void Actor::GetDirectionInfo(Pid uObj1ID, Pid uObj2ID,
                             AIDirection *pOut, int PreferedZ) {
    Vec3f out1;
    Vec3f out2;

    int id1 = uObj1ID.id();
    int id2 = uObj2ID.id();
    switch (uObj1ID.type()) {
        case OBJECT_Sprite: {
            out1 = pSpriteObjects[id1].vPosition;
            break;
        }
        case OBJECT_Actor: {
            out1 = (pActors[id1].pos + Vec3f(0, 0, pActors[id1].height * 0.75f));
            break;
        }
        case OBJECT_Character: {
            out1 = pParty->pos + Vec3f(0, 0, pParty->height / 3);
            if (id1 == 0) {
                // Do nothing.
            } else if (id1 == 4) {
                out1 += Vec3f::fromPolar(24, pParty->_viewYaw - TrigLUT.uIntegerHalfPi, 0);
            } else if (id1 == 3) {
                out1 += Vec3f::fromPolar(8, pParty->_viewYaw - TrigLUT.uIntegerHalfPi, 0);
            } else if (id1 == 2) {
                out1 += Vec3f::fromPolar(8, pParty->_viewYaw + TrigLUT.uIntegerHalfPi, 0);
            } else if (id1 == 1) {
                out1 += Vec3f::fromPolar(24, pParty->_viewYaw + TrigLUT.uIntegerHalfPi, 0);
            }
            break;
        }
        case OBJECT_Decoration: {
            out1 = pLevelDecorations[id1].vPosition;
            break;
        }
        case OBJECT_Face: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                out1 = pIndoor->faces[id1].boundingBox.center();
            }
            break;
        }
        default: {
            out1 = Vec3f();
            break;
        }
    }

    switch (uObj2ID.type()) {
        case OBJECT_Sprite: {
            out2 = pSpriteObjects[id2].vPosition;
            break;
        }
        case OBJECT_Actor: {
            out2 = (pActors[id2].pos + Vec3f(0, 0, pActors[id2].height * 0.75f));
            break;
        }
        case OBJECT_Character: {
            if (!PreferedZ)
                PreferedZ = pParty->eyeLevel;

            out2 = pParty->pos + Vec3f(0, 0, PreferedZ);
            break;
        }
        case OBJECT_Decoration: {
            out2 = pLevelDecorations[id2].vPosition;
            break;
        }
        case OBJECT_Face: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                out2 = pIndoor->faces[id2].boundingBox.center();
            }
            break;
        }
        default: {
            out2 = Vec3f();
            break;
        }
    }

    GetDirectionInfo(out1, out2, pOut);
}

//----- (00404030) --------------------------------------------------------
void Actor::AI_FaceObject(unsigned int uActorID, Pid uObjID,
                          AIDirection *Dir_In) {
    AIDirection *Dir_Out;
    AIDirection Dir_Ret;

    if (grng->random(100) >= 5) {
        if (!Dir_In) {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), uObjID, &Dir_Ret, 0);
            Dir_Out = &Dir_Ret;
        } else {
            Dir_Out = Dir_In;
        }

        pActors[uActorID].yawAngle = Dir_Out->uYawAngle;
        pActors[uActorID].currentActionTime = 0_ticks;
        pActors[uActorID].velocity.z = 0;
        pActors[uActorID].velocity.y = 0;
        pActors[uActorID].velocity.x = 0;
        pActors[uActorID].pitchAngle = Dir_Out->uPitchAngle;
        pActors[uActorID].currentActionLength = Duration::fromRealtimeSeconds(2);
        pActors[uActorID].aiState = Interacting;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Bored(uActorID, uObjID, Dir_In);
    }
}

//----- (00403F58) --------------------------------------------------------
void Actor::AI_StandOrBored(unsigned int uActorID, Pid uObjID,
                            Duration uActionLength, AIDirection *a4) {
    if (grng->random(2))  // 0 or 1
        AI_Bored(uActorID, uObjID, a4);
    else
        AI_Stand(uActorID, uObjID, uActionLength, a4);
}

//----- (00403EB6) --------------------------------------------------------
void Actor::AI_Stand(unsigned int uActorID, Pid object_to_face_pid,
                     Duration uActionLength, AIDirection *a4) {
    assert(uActorID < pActors.size());
    // Actor *actor = &pActors[uActorID];

    AIDirection a3;
    if (!a4) {
        Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), object_to_face_pid,
                                &a3, 0);
        a4 = &a3;
    }

    pActors[uActorID].aiState = Standing;
    if (!uActionLength)
        pActors[uActorID].currentActionLength = Duration::randomRealtimeSeconds(grng, 2, 4);
    else
        pActors[uActorID].currentActionLength = uActionLength;
    pActors[uActorID].currentActionTime = 0_ticks;
    pActors[uActorID].yawAngle = a4->uYawAngle;
    pActors[uActorID].pitchAngle = a4->uPitchAngle;
    pActors[uActorID].velocity.z = 0;
    pActors[uActorID].velocity.y = 0;
    pActors[uActorID].velocity.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403E61) --------------------------------------------------------
void Actor::StandAwhile(unsigned int uActorID) {
    pActors[uActorID].currentActionLength = Duration::randomRealtimeSeconds(grng, 1, 2);
    pActors[uActorID].currentActionTime = 0_ticks;
    pActors[uActorID].aiState = Standing;
    pActors[uActorID].velocity.z = 0;
    pActors[uActorID].velocity.y = 0;
    pActors[uActorID].velocity.x = 0;
    pActors[uActorID].UpdateAnimation();
}

//----- (00403C6C) --------------------------------------------------------
void Actor::AI_MeleeAttack(unsigned int uActorID, Pid sTargetPid,
                           AIDirection *arg0) {
    int v6;        // esi@6
    int16_t v7;        // edi@6
    signed int v8;     // eax@7
    Vec3f v10;     // ST04_12@9
    AIDirection *v12;  // eax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@12
    AIDirection v20;   // [sp+28h] [bp-2Ch]@12
    int v23;           // [sp+4Ch] [bp-8h]@6
    Duration v25;  // [sp+5Ch] [bp+8h]@13

    assert(uActorID < pActors.size());

    if (pActors[uActorID].monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY &&
        pActors[uActorID].monsterInfo.aiType == MONSTER_AI_WIMP) {
        Actor::AI_Stand(uActorID, sTargetPid, 0_ticks, arg0);
        return;
    }

    if (sTargetPid.type() == OBJECT_Actor) {
        v8 = sTargetPid.id();
        v6 = pActors[v8].pos.x;
        v7 = pActors[v8].pos.y;
        v23 = (int)(pActors[v8].height * 0.75 + pActors[v8].pos.z);
    } else if (sTargetPid.type() == OBJECT_Character) {
        v6 = pParty->pos.x;
        v7 = pParty->pos.y;
        v23 = pParty->pos.z + pParty->eyeLevel;
    } else {
        assert(false);
        return;
    }

    v10.x = pActors[uActorID].pos.x;
    v10.y = pActors[uActorID].pos.y;
    v10.z = pActors[uActorID].height * 0.75 + pActors[uActorID].pos.z;

    if (Check_LineOfSight(Vec3f(v6, v7, v23), v10)) {
        if (arg0 != nullptr) {
            v12 = arg0;
        } else {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v12 = &a3;
        }
        pActors[uActorID].yawAngle = (short)v12->uYawAngle;
        pActors[uActorID].currentActionLength =
            pSpriteFrameTable
                ->pSpriteSFrames[pActors[uActorID].spriteIds[ANIM_AtkMelee]]
                .animationLength;
        pActors[uActorID].currentActionTime = 0_ticks;
        pActors[uActorID].aiState = AttackingMelee;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        v25 = pMonsterStats->infos[pActors[uActorID].monsterInfo.id]
                  .recoveryTime;
        if (pActors[uActorID].buffs[ACTOR_BUFF_SLOWED].Active()) v25 *= 2;
        if (!pParty->bTurnBasedModeOn) {
            pActors[uActorID].monsterInfo.recoveryTime = debug_combat_recovery_mul * flt_debugrecmod3 * v25;
        } else {
            pActors[uActorID].monsterInfo.recoveryTime = v25;
        }
        pActors[uActorID].velocity.z = 0;
        pActors[uActorID].velocity.y = 0;
        pActors[uActorID].velocity.x = 0;
        pActors[uActorID].UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, grng->random(2), 64_ticks, arg0);
    }
}

//----- (00438CF3) --------------------------------------------------------
void Actor::ApplyFineForKillingPeasant(unsigned int uActorID) {
    if (engine->_currentLoadedMapId == MAP_INVALID || !pActors[uActorID].IsPeasant())
        return;

    if ((engine->_currentLoadedMapId == MAP_BRACADA_DESERT || engine->_currentLoadedMapId == MAP_CELESTE) && pParty->isPartyEvil())
        return;

    if ((engine->_currentLoadedMapId == MAP_DEYJA || engine->_currentLoadedMapId == MAP_PIT) && pParty->isPartyGood())
        return;

    pParty->uFine += 100 * (pMapStats->pInfos[engine->_currentLoadedMapId].baseStealingFine +
                            pActors[uActorID].monsterInfo.level +
                            pParty->GetPartyReputation());
    if (pParty->uFine < 0)
        pParty->uFine = 0;
    if (pParty->uFine > 4000000)
        pParty->uFine = 4000000;

    if (currentLocationInfo().reputation < 10000)
        currentLocationInfo().reputation++;

    if (pParty->uFine) {
        for (Character &character : pParty->pCharacters) {
            if (!character._achievedAwardsBits[AWARD_FINE]) {
                character._achievedAwardsBits.set(AWARD_FINE);
            }
        }
    }
}

//----- (0043AE80) --------------------------------------------------------
void Actor::AddOnDamageOverlay(unsigned int uActorID, int overlayType, signed int damage) {
    Pid actorPID = Pid(OBJECT_Actor, uActorID);
    switch (overlayType) {
        case 1:
            if (damage)
                pActiveOverlayList->_4418B6(904, actorPID, 0_ticks,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 2:
            if (damage)
                pActiveOverlayList->_4418B6(905, actorPID, 0_ticks,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 3:
            if (damage)
                pActiveOverlayList->_4418B6(906, actorPID, 0_ticks,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 4:
            if (damage)
                pActiveOverlayList->_4418B6(907, actorPID, 0_ticks,
                                           (int)(sub_43AE12(damage) * 65536.0), 0);
            return;
        case 5:
            pActiveOverlayList->_4418B6(901, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        case 6:
            pActiveOverlayList->_4418B6(902, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        case 7:
            pActiveOverlayList->_4418B6(903, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        case 8:
            pActiveOverlayList->_4418B6(900, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        case 9:
            pActiveOverlayList->_4418B6(909, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        case 10:
            pActiveOverlayList->_4418B6(908, actorPID, 0_ticks, 0, 0); // 4th param was actorPID.
            return;
        default:
            return;
    }
    return;
}

//----- (0043B3E0) --------------------------------------------------------
int Actor::_43B3E0_CalcDamage(ActorAbility dmgSource) {
    int damageDiceRolls;
    int damageDiceSides;
    int damageBonus;
    SpellId spellID;
    int spellPower = 0;
    CombinedSkillValue skill;
    int skillLevel = 0;
    Mastery skillMastery = MASTERY_NONE;

    switch (dmgSource) {
        case ABILITY_ATTACK1:
            if (this->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
                spellPower = this->buffs[ACTOR_BUFF_HOUR_OF_POWER].power;
            if (this->buffs[ACTOR_BUFF_HEROISM].Active() && this->buffs[ACTOR_BUFF_HEROISM].power > spellPower)
                spellPower = this->buffs[ACTOR_BUFF_HEROISM].power;
            if (this->buffs[ACTOR_BUFF_HAMMERHANDS].Active())
                spellPower += this->buffs[ACTOR_BUFF_HAMMERHANDS].power;
            damageDiceRolls = this->monsterInfo.attack1DamageDiceRolls;
            damageDiceSides = this->monsterInfo.attack1DamageDiceSides;
            damageBonus = this->monsterInfo.attack1DamageBonus;
            break;
        case ABILITY_ATTACK2:
            damageDiceRolls = this->monsterInfo.attack2DamageDiceRolls;
            damageDiceSides = this->monsterInfo.attack2DamageDiceSides;
            damageBonus = this->monsterInfo.attack2DamageBonus;
            break;
        case ABILITY_SPELL1:
            spellID = this->monsterInfo.spell1Id;
            skill = this->monsterInfo.spell1SkillMastery;
            skillLevel = skill.level();
            skillMastery = skill.mastery();
            return CalcSpellDamage(spellID, skillLevel, skillMastery, 0);
            break;
        case ABILITY_SPELL2:
            spellID = this->monsterInfo.spell2Id;
            skill = this->monsterInfo.spell2SkillMastery;
            skillLevel = skill.level();
            skillMastery = skill.mastery();
            return CalcSpellDamage(spellID, skillLevel, skillMastery, 0);
            break;
        case ABILITY_SPECIAL:
            damageDiceRolls = this->monsterInfo.specialAbilityDamageDiceRolls;
            damageDiceSides = this->monsterInfo.specialAbilityDamageDiceSides;
            damageBonus = this->monsterInfo.specialAbilityDamageDiceBonus;
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
    MonsterType monsterType = this->ally;
    if (this->ally == MONSTER_TYPE_INVALID)
        monsterType = monsterTypeForMonsterId(this->monsterInfo.id);
    return isPeasant(monsterType);
}

//----- (0042EBEE) --------------------------------------------------------
void Actor::StealFrom(unsigned int uActorID) {
    Character *pPlayer;     // edi@1
    int v4 = 0;              // ebx@2
    LocationInfo *v6;  // esi@4
    Duration v8;              // [sp+8h] [bp-4h]@6

    pPlayer = &pParty->pCharacters[pParty->activeCharacterIndex() - 1];
    if (pPlayer->CanAct()) {
        CastSpellInfoHelpers::cancelSpellCastInProgress();
        if (engine->_currentLoadedMapId != MAP_INVALID)
            v4 = pMapStats->pInfos[engine->_currentLoadedMapId].baseStealingFine;
        v6 = &currentLocationInfo();
        pPlayer->StealFromActor(uActorID, v4, v6->reputation++);
        v8 = pPlayer->GetAttackRecoveryTime(false);
        v8 = std::max(v8, Duration::fromTicks(engine->config->gameplay.MinRecoveryMelee.value()));
        if (!pParty->bTurnBasedModeOn)
            pPlayer->SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * v8);
        pTurnEngine->ApplyPlayerAction();
    }
}

//----- (00403A60) --------------------------------------------------------
void Actor::AI_SpellAttack2(unsigned int uActorID, Pid edx0,
                            AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3f v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    Duration v13;         // ax@10
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v18;     // [sp+28h] [bp-2Ch]@9
    int v19;             // [sp+44h] [bp-10h]@6
    Pid a2;       // [sp+48h] [bp-Ch]@1
    int v21;             // [sp+4Ch] [bp-8h]@3
    Duration pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    a2 = edx0;
    if (edx0.type() == OBJECT_Actor) {
        v6 = edx0.id();
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v21 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (edx0.type() == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v21 = pParty->pos.z + pParty->eyeLevel;
    } else {
        assert(false);
        return;
    }
    v19 = v3->height;
    v7.z = v3->pos.z + (v19 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3f(v4, v5, v21), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), a2, &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .animationLength;
        v3->currentActionLength = v13;
        v3->currentActionTime = 0_ticks;
        v3->aiState = AttackingRanged4;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->infos[v3->monsterInfo.id].recoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.recoveryTime = pDira;
        } else {
            v3->monsterInfo.recoveryTime = v3->currentActionLength + debug_combat_recovery_mul * flt_debugrecmod3 * pDira;
        }
        v3->velocity.z = 0;
        v3->velocity.y = 0;
        v3->velocity.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->monsterInfo.spell2Id)) {
            v3->currentActionLength = 64_ticks;
            v3->currentActionTime = 0_ticks;
            v3->aiState = Fidgeting;
            v3->UpdateAnimation();
            v3->aiState = AttackingRanged4;
        } else {
            v3->UpdateAnimation();
        }
    } else {
        Actor::AI_Pursue1(uActorID, a2, uActorID, 64_ticks, pDir);
    }
}

//----- (00403854) --------------------------------------------------------
void Actor::AI_SpellAttack1(unsigned int uActorID, Pid sTargetPid,
                            AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3f v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    Duration v13;         // ax@10
    SpellId v16;      // ecx@17
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v18;     // [sp+28h] [bp-2Ch]@9
    int v19;             // [sp+44h] [bp-10h]@6
    int v21;             // [sp+4Ch] [bp-8h]@3
    Duration pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (sTargetPid.type() == OBJECT_Actor) {
        v6 = sTargetPid.id();
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v21 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (sTargetPid.type() == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v21 = pParty->pos.z + pParty->eyeLevel;
    } else {
        assert(false);
        return;
    }
    v19 = v3->height;
    v7.z = v3->pos.z + (v19 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3f(v4, v5, v21), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .animationLength;
        v3->currentActionLength = v13;
        v3->currentActionTime = 0_ticks;
        v3->aiState = AttackingRanged3;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->infos[v3->monsterInfo.id].recoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.recoveryTime = pDira;
        } else {
            v3->monsterInfo.recoveryTime = v3->currentActionLength + debug_combat_recovery_mul * flt_debugrecmod3 * pDira;
        }
        v16 = v3->monsterInfo.spell1Id;
        v3->velocity.z = 0;
        v3->velocity.y = 0;
        v3->velocity.x = 0;
        if (ShouldMonsterPlayAttackAnim(v3->monsterInfo.spell1Id)) {
            v3->currentActionLength = 64_ticks;
            v3->currentActionTime = 0_ticks;
            v3->aiState = Fidgeting;
            v3->UpdateAnimation();
            v3->aiState = AttackingRanged3;
        } else {
            v3->UpdateAnimation();
        }
    } else {
        // TODO(pskelton): Consider adding potshots if no LOS
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64_ticks, pDir);
    }
}

//----- (0040368B) --------------------------------------------------------
void Actor::AI_MissileAttack2(unsigned int uActorID, Pid sTargetPid,
                              AIDirection *pDir) {
    Actor *v3;           // ebx@1
    int16_t v4;          // esi@3
    int16_t v5;          // edi@3
    signed int v6;       // eax@4
    Vec3f v7;        // ST04_12@6
    AIDirection *v9;     // eax@8
    Duration v13;         // ax@10
    AIDirection a3;      // [sp+Ch] [bp-48h]@9
    AIDirection v17;     // [sp+28h] [bp-2Ch]@9
    int v18;             // [sp+44h] [bp-10h]@6
    int v20;             // [sp+4Ch] [bp-8h]@3
    Duration pDira;  // [sp+5Ch] [bp+8h]@10

    v3 = &pActors[uActorID];
    if (sTargetPid.type() == OBJECT_Actor) {
        v6 = sTargetPid.id();
        v4 = pActors[v6].pos.x;
        v5 = pActors[v6].pos.y;
        v20 = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else if (sTargetPid.type() == OBJECT_Character) {
        v4 = pParty->pos.x;
        v5 = pParty->pos.y;
        v20 = pParty->pos.z + pParty->eyeLevel;
    } else {
        assert(false);
        return;
    }
    v18 = v3->height;
    v7.z = v3->pos.z + (v18 * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3f(v4, v5, v20), v7)) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v9 = &a3;
        } else {
            v9 = pDir;
        }
        v3->yawAngle = (short)v9->uYawAngle;
        v13 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .animationLength;
        v3->currentActionLength = v13;
        v3->currentActionTime = 0_ticks;
        v3->aiState = AttackingRanged2;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->infos[v3->monsterInfo.id].recoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (!pParty->bTurnBasedModeOn) {
            v3->monsterInfo.recoveryTime = debug_combat_recovery_mul * flt_debugrecmod3 * pDira;
        } else {
            v3->monsterInfo.recoveryTime = pDira;
        }
        v3->velocity.z = 0;
        v3->velocity.y = 0;
        v3->velocity.x = 0;
        v3->UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64_ticks, pDir);
    }
}

//----- (00403476) --------------------------------------------------------
void Actor::AI_MissileAttack1(unsigned int uActorID, Pid sTargetPid,
                              AIDirection *pDir) {
    Actor *v3;         // ebx@1
    int xpos;            // esi@3
    int ypos;            // edi@3
    signed int v6;     // eax@4
    Vec3f v7;      // ST04_12@6
    AIDirection *v10;  // eax@9
    Duration v14;       // ax@11
    AIDirection a3;    // [sp+Ch] [bp-48h]@10
    AIDirection v18;   // [sp+28h] [bp-2Ch]@10
    //int v19;           // [sp+44h] [bp-10h]@6
    // signed int a2; // [sp+48h] [bp-Ch]@1
    int zpos = 0;             // [sp+50h] [bp-4h]@3
    Duration pDira;  // [sp+5Ch] [bp+8h]@11

    v3 = &pActors[uActorID];
    // a2 = edx0;
    if (sTargetPid.type() == OBJECT_Actor) {
        v6 = sTargetPid.id();
        xpos = pActors[v6].pos.x;
        ypos = pActors[v6].pos.y;
        zpos = (int)(pActors[v6].height * 0.75 + pActors[v6].pos.z);
    } else {
        if (sTargetPid.type() == OBJECT_Character) {
            xpos = pParty->pos.x;
            ypos = pParty->pos.y;
            zpos = pParty->pos.z + pParty->eyeLevel;
        } else {
            xpos = pDir->vDirection.x * 65536; // make a target point at distance
            ypos = pDir->vDirection.y * 65536;
        }
    }
    //v19 = v3->uActorHeight;
    v7.z = v3->pos.z + (v3->height * 0.75);
    v7.y = v3->pos.y;
    v7.x = v3->pos.x;
    if (Check_LineOfSight(Vec3f(xpos, ypos, zpos), v7)
        // || Check_LineOfSight(v7.x, v7.y, v7.z, Vec3f(xpos, ypos, zpos))
        ) {
        if (pDir == nullptr) {
            Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), sTargetPid,
                                    &a3, 0);
            v10 = &a3;
        } else {
            v10 = pDir;
        }
        v3->yawAngle = (short)v10->uYawAngle;
        v14 = pSpriteFrameTable->pSpriteSFrames[v3->spriteIds[ANIM_AtkRanged]]
                  .animationLength;
        v3->currentActionLength = v14;
        v3->currentActionTime = 0_ticks;
        v3->aiState = AttackingRanged1;
        Actor::playSound(uActorID, ACTOR_ATTACK_SOUND);
        pDira = pMonsterStats->infos[v3->monsterInfo.id].recoveryTime;
        if (v3->buffs[ACTOR_BUFF_SLOWED].Active()) pDira *= 2;
        if (pParty->bTurnBasedModeOn) {
            v3->monsterInfo.recoveryTime = pDira;
        } else {
            v3->monsterInfo.recoveryTime = v3->currentActionLength + flt_debugrecmod3 * debug_combat_recovery_mul * pDira;
        }
        v3->velocity.z = 0;
        v3->velocity.y = 0;
        v3->velocity.x = 0;
        v3->UpdateAnimation();
    } else {
        Actor::AI_Pursue1(uActorID, sTargetPid, uActorID, 64_ticks, pDir);
    }
}

//----- (004032B2) --------------------------------------------------------
void Actor::AI_RandomMove(unsigned int uActor_id, Pid uTarget_id,
                          int radius, Duration uActionLength) {
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
    absx = std::abs(x);
    absy = std::abs(y);
    if (absx <= absy)
        absx = absy + (absx / 2);
    else
        absx = absx + absy / 2;
    if (supertypeForMonsterId(pActors[uActor_id].monsterInfo.id) == MONSTER_SUPERTYPE_TREANT) {
        if (!uActionLength) uActionLength = 256_ticks;
        Actor::AI_StandOrBored(uActor_id, Pid(OBJECT_Character, 0), uActionLength,
                               &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    if (pActors[uActor_id].monsterInfo.movementType ==
        MONSTER_MOVEMENT_TYPE_GLOBAL &&
        absx < 128) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256_ticks,
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
    if (std::abs(v10 - pActors[uActor_id].yawAngle) > 256 &&
        !(pActors[uActor_id].attributes & ACTOR_ANIMATION)) {
        Actor::AI_Stand(uActor_id, uTarget_id, 256_ticks,
                        &doNotInitializeBecauseShouldBeRandom);
        return;
    }
    pActors[uActor_id].yawAngle = v10;
    if (pActors[uActor_id].moveSpeed)
        pActors[uActor_id].currentActionLength = Duration::fromTicks(
            32 * absx / pActors[uActor_id].moveSpeed);
    else
        pActors[uActor_id].currentActionLength = 0_ticks;
    pActors[uActor_id].currentActionTime = 0_ticks;
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
void Actor::AI_Stun(unsigned int uActorID, Pid edx0,
                    int stunRegardlessOfState) {
    Duration v7;      // ax@16
    AIDirection a3;  // [sp+Ch] [bp-40h]@16

    if (pActors[uActorID].aiState == Fleeing)
        pActors[uActorID].attributes |= ACTOR_FLEEING;
    if (pActors[uActorID].monsterInfo.hostilityType != HOSTILITY_LONG) {
        pActors[uActorID].attributes &= ~ACTOR_UNKNOWN_4;
        pActors[uActorID].monsterInfo.hostilityType = HOSTILITY_LONG;
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
        Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), edx0, &a3, 0);
        // v10 = &a3;
        pActors[uActorID].yawAngle = (short)a3.uYawAngle;
        v7 = pSpriteFrameTable
                 ->pSpriteSFrames[pActors[uActorID].spriteIds[ANIM_GotHit]]
                 .animationLength;
        pActors[uActorID].currentActionTime = 0_ticks;
        pActors[uActorID].aiState = Stunned;
        pActors[uActorID].currentActionLength = v7;
        Actor::playSound(uActorID, ACTOR_STUNNED_SOUND);
        pActors[uActorID].UpdateAnimation();
    }
}

//----- (00402F87) --------------------------------------------------------
void Actor::AI_Bored(unsigned int uActorID, Pid uObjID,
                     AIDirection *a4) {
    unsigned int v7;  // eax@3
    unsigned int v9;  // eax@3

    Actor *actor = &pActors[uActorID];

    AIDirection a3;  // [sp+Ch] [bp-5Ch]@2
    if (!a4) {
        Actor::GetDirectionInfo(Pid(OBJECT_Actor, uActorID), uObjID, &a3, 0);
        a4 = &a3;
    }

    actor->currentActionLength =
        pSpriteFrameTable->pSpriteSFrames[actor->spriteIds[ANIM_Bored]]
                .animationLength;

    v7 = TrigLUT.atan2(actor->pos.x - pCamera3D->vCameraPos.x, actor->pos.y - pCamera3D->vCameraPos.y);
    v9 = TrigLUT.uIntegerPi + actor->yawAngle +
         ((signed int)TrigLUT.uIntegerPi >> 3) - v7;

    if (v9 & 0x700) {  // turned away - just stand
        Actor::AI_Stand(uActorID, uObjID, actor->currentActionLength, a4);
    } else {  // facing player - play bored anim
        actor->aiState = Fidgeting;
        actor->currentActionTime = 0_ticks;
        actor->yawAngle = a4->uYawAngle;
        actor->velocity.z = 0;
        actor->velocity.y = 0;
        actor->velocity.x = 0;
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
    pActor->currentActionTime = 0_ticks;
    pActor->aiState = Resurrected;
    pActor->currentActionAnimation = ANIM_Dying;
    pActor->currentActionLength =
        pSpriteFrameTable->pSpriteSFrames[pActor->spriteIds[ANIM_Dying]]
                .animationLength;
    pActor->hp = (short)pActor->monsterInfo.hp;
    Actor::playSound(uActorID, ACTOR_DEATH_SOUND);
    pActor->UpdateAnimation();

    pActor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
    // TODO(pskelton): vanilla behaviour but does it make sense to drop all carried treasure
    pActor->monsterInfo.treasureDropChance = 0;
    pActor->monsterInfo.goldDiceRolls = 0;
    pActor->monsterInfo.goldDiceSides = 0;
    pActor->monsterInfo.treasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    pActor->monsterInfo.treasureType = RANDOM_ITEM_ANY;
    pActor->ally = MONSTER_TYPE_9999;
    pActor->ResetAggressor();  // ~0x80000
    pActor->group = 0;
    pActor->buffs[ACTOR_BUFF_BERSERK].Reset();
    pActor->buffs[ACTOR_BUFF_CHARM].Reset();
    pActor->buffs[ACTOR_BUFF_ENSLAVED].Reset();
}

//----- (00402D6E) --------------------------------------------------------
void Actor::Die(unsigned int uActorID) {
    Actor *actor = &pActors[uActorID];

    actor->currentActionTime = 0_ticks;
    actor->aiState = Dying;
    actor->currentActionAnimation = ANIM_Dying;
    actor->hp = 0;
    actor->currentActionLength =
        pSpriteFrameTable->pSpriteSFrames[actor->spriteIds[ANIM_Dying]]
                .animationLength;
    actor->buffs[ACTOR_BUFF_PARALYZED].Reset();
    actor->buffs[ACTOR_BUFF_STONED].Reset();
    Actor::playSound(uActorID, ACTOR_DEATH_SOUND);
    actor->UpdateAnimation();

    for (HouseId house : allTownhallHouses())
        if (pParty->monster_id_for_hunting[house] == actor->monsterInfo.id)
            pParty->monster_for_hunting_killed[house] = true;

    for (SpellBuff &buff : actor->buffs)
        buff.Reset();

    Item drop;
    drop.Reset();
    drop.itemId = itemDropForMonsterType(monsterTypeForMonsterId(actor->monsterInfo.id));

    if (grng->random(100) < 20 && drop.itemId != ITEM_NULL) {
        SpriteObject::dropItemAt(pItemTable->items[drop.itemId].spriteId,
                                 actor->pos + Vec3f(0, 0, 16), grng->random(200) + 200, 1, true, 0, &drop);
    }

    if (actor->monsterInfo.specialAbilityType == MONSTER_SPECIAL_ABILITY_EXPLODE) {
        Actor::Explode(uActorID);
    }
}

void Actor::playSound(unsigned int uActorID, ActorSound uSoundID) {
    SoundId sound_sample_id = pActors[uActorID].soundSampleIds[uSoundID];
    if (sound_sample_id != SOUND_Invalid) {
        if (!pActors[uActorID].buffs[ACTOR_BUFF_SHRINK].Active()) {
            pAudioPlayer->playSound(sound_sample_id, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
        } else {
            // TODO(pskelton): looks incomplete - sounds meant to change depending on actor size
            switch (pActors[uActorID].buffs[ACTOR_BUFF_SHRINK].power) {
                case 1:
                    pAudioPlayer->playSound(sound_sample_id, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
                    break;
                case 2:
                    pAudioPlayer->playSound(sound_sample_id, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
                    break;
                case 3:
                case 4:
                    pAudioPlayer->playSound(sound_sample_id, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
                    break;
                default:
                    pAudioPlayer->playSound(sound_sample_id, SOUND_MODE_PID, Pid(OBJECT_Actor, uActorID));
                    break;
            }
        }
    }
}

//----- (00402AD7) --------------------------------------------------------
void Actor::AI_Pursue1(unsigned int uActorID, Pid a2, signed int arg0,
                       Duration uActionLength, AIDirection *pDir) {
    Actor *v7;         // ebx@1
    Pid v8;   // ecx@1
    AIDirection *v10;  // esi@6
    AIDirection a3;    // [sp+Ch] [bp-5Ch]@7
    unsigned int v18;  // [sp+64h] [bp-4h]@1

    int WantedZ = 0;
    v7 = &pActors[uActorID];
    v8 = Pid(OBJECT_Actor, uActorID);
    if (v7->monsterInfo.flying && !pParty->bFlying) {
        if (v7->monsterInfo.attack1MissileType != MONSTER_PROJECTILE_NONE)
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
    if (supertypeForMonsterId(v7->monsterInfo.id) == MONSTER_SUPERTYPE_TREANT) {
        if (!uActionLength) uActionLength = 256_ticks;
        Actor::AI_StandOrBored(uActorID, Pid::character(0), uActionLength, v10);
        return;
    }
    if (v10->uDistance < meleeRange) {
        if (!uActionLength) uActionLength = 256_ticks;
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
        v7->currentActionLength = 128_ticks;
    v7->pitchAngle = (short)v10->uPitchAngle;
    v7->aiState = Pursuing;
    v7->UpdateAnimation();
}

//----- (00402968) --------------------------------------------------------
void Actor::AI_Flee(unsigned int uActorID, Pid sTargetPid,
                    Duration uActionLength, AIDirection *a4) {
    Actor *v5;         // ebx@1
    Pid v7;            // ecx@2
    AIDirection v10;   // [sp+8h] [bp-7Ch]@4
    AIDirection a3;    // [sp+24h] [bp-60h]@3
    AIDirection *v13;  // [sp+5Ch] [bp-28h]@4

    v5 = &pActors[uActorID];
    if (v5->CanAct()) {
        v7 = Pid(OBJECT_Actor, uActorID);
        if (!a4) {
            Actor::GetDirectionInfo(v7, sTargetPid, &a3, 0);
            a4 = &a3;
        }
        Actor::GetDirectionInfo(v7, Pid::character(0), &v10, 0);
        v13 = &v10;
        if (supertypeForMonsterId(v5->monsterInfo.id) == MONSTER_SUPERTYPE_TREANT ||
            sTargetPid.type() == OBJECT_Actor && v13->uDistance < meleeRange) {
            if (!uActionLength) uActionLength = 256_ticks;
            Actor::AI_StandOrBored(uActorID, Pid::character(0), uActionLength, v13);
        } else {
            if (v5->moveSpeed)
                v5->currentActionLength = Duration::fromTicks(
                    (signed int)(a4->uDistanceXZ * 128) / v5->moveSpeed);
            else
                v5->currentActionLength = 0_ticks;
            if (v5->currentActionLength > 256_ticks) v5->currentActionLength = 256_ticks;
            v5->yawAngle =
                (short)TrigLUT.uIntegerHalfPi + (short)a4->uYawAngle;
            v5->yawAngle =
                (short)TrigLUT.uDoublePiMask &
                (v5->yawAngle + grng->random(TrigLUT.uIntegerPi));
            v5->currentActionTime = 0_ticks;
            v5->pitchAngle = (short)a4->uPitchAngle;
            v5->aiState = Fleeing;
            v5->UpdateAnimation();
        }
    }
}

//----- (0040281C) --------------------------------------------------------
void Actor::AI_Pursue2(unsigned int uActorID, Pid a2,
                       Duration uActionLength, AIDirection *pDir, int a5) {
    int v6;                // eax@1
    Actor *v7;             // ebx@1
    Pid v8;       // ecx@1
    AIDirection *v10;      // esi@7
    int16_t v13;    // cx@19
    uint16_t v14;  // ax@25
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection v18;       // [sp+28h] [bp-24h]@8

    v6 = 0;
    v7 = &pActors[uActorID];
    v8 = Pid(OBJECT_Actor, uActorID);
    if (v7->monsterInfo.flying && !pParty->bFlying) {
        if (v7->monsterInfo.attack1MissileType != MONSTER_PROJECTILE_NONE &&
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
    if (supertypeForMonsterId(v7->monsterInfo.id) == MONSTER_SUPERTYPE_TREANT) {
        if (!uActionLength) uActionLength = 256_ticks;
        Actor::AI_StandOrBored(uActorID, Pid::character(0), uActionLength, v10);
        return;
    }
    if ((signed int)v10->uDistance < a5) {
        if (!uActionLength) uActionLength = 256_ticks;
        Actor::AI_StandOrBored(uActorID, a2, uActionLength, v10);
        return;
    }
    if (uActionLength) {
        v7->currentActionLength = uActionLength;
    } else {
        v13 = v7->moveSpeed;
        if (v13)
            v7->currentActionLength = Duration::fromTicks(
                (signed int)(v10->uDistanceXZ * 128) / v13);
        else
            v7->currentActionLength = 0_ticks;
        if (v7->currentActionLength > 32_ticks) v7->currentActionLength = 32_ticks;
    }
    v7->yawAngle = (short)v10->uYawAngle;
    v14 = (short)v10->uPitchAngle;
    v7->currentActionTime = 0_ticks;
    v7->pitchAngle = v14;
    v7->aiState = Pursuing;
    v7->UpdateAnimation();
}

//----- (00402686) --------------------------------------------------------
void Actor::AI_Pursue3(unsigned int uActorID, Pid a2,
                       Duration uActionLength, AIDirection *a4) {
    int v5;                // eax@1
    Actor *v6;             // ebx@1
    Pid v7;                // ecx@1
    int16_t v12;    // cx@19
    int16_t v14;           // ax@25
    uint16_t v16;  // ax@28
    AIDirection a3;        // [sp+Ch] [bp-40h]@8
    AIDirection *v20;      // [sp+28h] [bp-24h]@8

    v5 = 0;
    v6 = &pActors[uActorID];
    v7 = Pid(OBJECT_Actor, uActorID);
    if (v6->monsterInfo.flying && !pParty->bFlying) {
        if (v6->monsterInfo.attack1MissileType != MONSTER_PROJECTILE_NONE &&
            uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
            v5 = v6->radius + 512;
        else
            v5 = pParty->height;
    }
    if (!a4) {
        Actor::GetDirectionInfo(v7, a2, &a3, v5);
        v20 = &a3;
    }
    if (supertypeForMonsterId(v6->monsterInfo.id) == MONSTER_SUPERTYPE_TREANT) {
        if (!uActionLength) uActionLength = 256_ticks;
        return Actor::AI_StandOrBored(uActorID, Pid::character(0), uActionLength, a4);
    }
    if (a4->uDistance < meleeRange) {
        if (!uActionLength) uActionLength = 256_ticks;
        return Actor::AI_StandOrBored(uActorID, a2, uActionLength, a4);
    }
    if (uActionLength) {
        v6->currentActionLength = uActionLength + Duration::random(grng, uActionLength);
    } else {
        v12 = v6->moveSpeed;
        if (v12)
            v6->currentActionLength = Duration::fromTicks((a4->uDistanceXZ * 128) / v12);
        else
            v6->currentActionLength = 0_ticks;
        if (v6->currentActionLength > 128_ticks) v6->currentActionLength = 128_ticks;
    }
    v14 = (short)a4->uYawAngle;
    if (grng->random(2))
        v14 += 256;
    else
        v14 -= 256;
    v6->yawAngle = v14;
    v16 = (short)a4->uPitchAngle;
    v6->currentActionTime = 0_ticks;
    v6->pitchAngle = v16;
    v6->aiState = Pursuing;
    if (vrng->random(100) < 2) {
        Actor::playSound(uActorID, ACTOR_STUNNED_SOUND);
    }
    v6->UpdateAnimation();
}

//----- (00401221) --------------------------------------------------------
void Actor::_SelectTarget(unsigned int uActorID, Pid *OutTargetPID,
                          bool can_target_party) {
    int v5;                     // ecx@1
    MonsterHostility v10;             // eax@13
    unsigned v11;                   // ebx@16
    unsigned v12;                   // eax@16
    MonsterHostility v14;             // eax@31
    unsigned v15;                   // edi@43
    //unsigned v16;                   // ebx@45
    //unsigned v17;                   // eax@45
    signed int closestId;       // [sp+14h] [bp-1Ch]@1
    unsigned v23;                   // [sp+1Ch] [bp-14h]@16
    unsigned int lowestRadius;  // [sp+24h] [bp-Ch]@1
    unsigned v27;                   // [sp+2Ch] [bp-4h]@16
    //unsigned v28;                   // [sp+2Ch] [bp-4h]@45

    lowestRadius = UINT_MAX;
    v5 = 0;
    // TODO(pskelton): change to PID_INVALID and sort logic in calling funcs
    *OutTargetPID = Pid();
    closestId = 0;
    assert(uActorID < pActors.size());
    Actor *thisActor = &pActors[uActorID];

    for (unsigned i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];
        if (actor->aiState == Dead || actor->aiState == Dying ||
            actor->aiState == Removed || actor->aiState == Summoned ||
            actor->aiState == Disabled || uActorID == i)
            continue;

        if (!thisActor->lastCharacterIdToHit || Pid(OBJECT_Actor, v5) != thisActor->lastCharacterIdToHit) {
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == HOSTILITY_FRIENDLY) continue;
        } else if (thisActor->IsNotAlive()) {
            thisActor->lastCharacterIdToHit = Pid();
            v10 = thisActor->GetActorsRelation(actor);
            if (v10 == HOSTILITY_FRIENDLY) continue;
        } else {
            if ((actor->group != 0 || thisActor->group != 0) &&
                actor->group == thisActor->group)
                continue;
            v10 = HOSTILITY_LONG;
        }
        if (thisActor->monsterInfo.hostilityType != HOSTILITY_FRIENDLY)
            v10 = pMonsterStats->infos[thisActor->monsterInfo.id]
                      .hostilityType;
        v11 = _4DF380_hostilityRanges[v10];
        v23 = std::abs(thisActor->pos.x - actor->pos.x);
        v27 = std::abs(thisActor->pos.y - actor->pos.y);
        v12 = std::abs(thisActor->pos.z - actor->pos.z);
        if (v23 <= v11 && v27 <= v11 && v12 <= v11 &&
            Detect_Between_Objects(Pid(OBJECT_Actor, i),
                                            Pid(OBJECT_Actor, uActorID)) &&
            v23 * v23 + v27 * v27 + v12 * v12 < lowestRadius) {
            lowestRadius = v23 * v23 + v27 * v27 + v12 * v12;
            closestId = i;
        }
    }

    if (lowestRadius != UINT_MAX) {
        *OutTargetPID = Pid(OBJECT_Actor, closestId);
    }

    if (can_target_party && !pParty->Invisible()) {
        if (thisActor->ActorEnemy() &&
            !thisActor->buffs[ACTOR_BUFF_ENSLAVED].Active() &&
            !thisActor->buffs[ACTOR_BUFF_CHARM].Active() &&
            !thisActor->buffs[ACTOR_BUFF_SUMMONED].Active())
            v14 = HOSTILITY_LONG;
        else
            v14 = thisActor->GetActorsRelation(0);
        if (v14 != HOSTILITY_FRIENDLY) {
            if (thisActor->monsterInfo.hostilityType == HOSTILITY_FRIENDLY)
                v15 = _4DF380_hostilityRanges[v14];
            else
                v15 = _4DF380_hostilityRanges[HOSTILITY_LONG];
            unsigned v16 = std::abs(thisActor->pos.x - pParty->pos.x);
            unsigned v28 = std::abs(thisActor->pos.y - pParty->pos.y);
            unsigned v17 = std::abs(thisActor->pos.z - pParty->pos.z);
            if (v16 <= v15 && v28 <= v15 && v17 <= v15 &&
                (v16 * v16 + v28 * v28 + v17 * v17 < lowestRadius)) {
                *OutTargetPID = Pid(OBJECT_Character, 0);
            }
        }
    }
}

//----- (0040104C) --------------------------------------------------------
MonsterHostility Actor::GetActorsRelation(Actor *otherActPtr) {
    MonsterType thisGroup;  // ebp@19
    MonsterType otherGroup;          // eax@22
    MonsterType thisAlly;   // edx@25
    MonsterType otherAlly;  // edx@33

    if (otherActPtr) {
        if (otherActPtr->group != 0 && this->group != 0 &&
            otherActPtr->group == this->group)
            return HOSTILITY_FRIENDLY;
    }

    if (this->buffs[ACTOR_BUFF_BERSERK].Active()) return HOSTILITY_LONG;
    thisAlly = this->ally;
    if (this->buffs[ACTOR_BUFF_ENSLAVED].Active() || thisAlly == MONSTER_TYPE_9999)
        thisGroup = MONSTER_TYPE_INVALID;
    else if (thisAlly != MONSTER_TYPE_INVALID)
        thisGroup = thisAlly;
    else
        thisGroup = monsterTypeForMonsterId(this->monsterInfo.id);

    if (otherActPtr) {
        if (otherActPtr->buffs[ACTOR_BUFF_BERSERK].Active()) return HOSTILITY_LONG;
        otherAlly = otherActPtr->ally;
        if (otherActPtr->buffs[ACTOR_BUFF_ENSLAVED].Active() || otherAlly == MONSTER_TYPE_9999)
            otherGroup = MONSTER_TYPE_INVALID;
        else if (otherAlly != MONSTER_TYPE_INVALID)
            otherGroup = otherAlly;
        else
            otherGroup = monsterTypeForMonsterId(otherActPtr->monsterInfo.id);
    } else {
        otherGroup = MONSTER_TYPE_INVALID;
    }

    if (this->buffs[ACTOR_BUFF_CHARM].Active() && otherGroup == MONSTER_TYPE_INVALID ||
        otherActPtr && otherActPtr->buffs[ACTOR_BUFF_CHARM].Active() && thisGroup == MONSTER_TYPE_INVALID)
        return HOSTILITY_FRIENDLY;
    if (!this->buffs[ACTOR_BUFF_ENSLAVED].Active() && this->ActorEnemy() && otherGroup == MONSTER_TYPE_INVALID)
        return HOSTILITY_LONG;
    if (thisGroup > MONSTER_TYPE_LAST || otherGroup > MONSTER_TYPE_LAST) return HOSTILITY_FRIENDLY;

    if (thisGroup == MONSTER_TYPE_INVALID) {
        if ((!otherActPtr || this->buffs[ACTOR_BUFF_ENSLAVED].Active() &&
                             otherActPtr->ActorFriend()) &&
            pFactionTable->relations[otherGroup][MONSTER_TYPE_INVALID] == HOSTILITY_FRIENDLY)
            return pFactionTable->relations[MONSTER_TYPE_INVALID][otherGroup];
        else
            return HOSTILITY_LONG;
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
            if (pSpriteFrameTable->pSpriteSFrames[spriteIds[ANIM_Dead]].sprites[0] == nullptr)
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

    v3 = &pMonsterList->monsters[monsterInfo.id];
    v9 = &pMonsterStats->infos[monsterInfo.id /*- 1 + 1*/];
    // v12 = pSpriteIDs;
    // Source = (char *)v3->pSpriteNames;
    // do
    for (ActorAnimation i : spriteIds.indices()) {
        // strcpy(pSpriteName, v3->pSpriteNames[i]);
        spriteIds[i] = pSpriteFrameTable->FastFindSprite(v3->spriteNames[i]);
        pSpriteFrameTable->InitializeSprite(spriteIds[i]);
    }
    height = v3->monsterHeight;
    radius = v3->monsterRadius;
    moveSpeed = v9->baseSpeed;
    if (!(load_sounds_if_bit1_set & 1)) {
        soundSampleIds = v3->soundSampleIds;
    }
}

//----- (00459667) --------------------------------------------------------
void Actor::Remove() { this->aiState = Removed; }

//----- (0043B1B0) --------------------------------------------------------
void Actor::ActorDamageFromMonster(Pid attacker_id,
                                   unsigned int actor_id, const Vec3f &pVelocity,
                                   ActorAbility a4) {
    int v4;            // ebx@1
    int dmgToRecv;     // qax@8
    DamageType v12;    // ecx@20
    int finalDmg;      // edi@30
    int pushDistance;  // [sp+20h] [bp+Ch]@34

    v4 = 0;
    if (attacker_id.type() == OBJECT_Sprite) {
        v4 = pSpriteObjects[attacker_id.id()]
                 .field_60_distance_related_prolly_lod;
        attacker_id = pSpriteObjects[attacker_id.id()].spell_caster_pid;
    }
    if (attacker_id.type() == OBJECT_Actor) {
        if (!pActors[actor_id].IsNotAlive()) {
            pActors[actor_id].lastCharacterIdToHit = attacker_id;
            if (pActors[actor_id].aiState == Fleeing)
                pActors[actor_id].attributes |= ACTOR_FLEEING;
            if (pActors[attacker_id.id()]._4273BB_DoesHitOtherActor(
                    &pActors[actor_id], v4, 0)) {
                dmgToRecv = pActors[attacker_id.id()]._43B3E0_CalcDamage(a4);
                if (pActors[attacker_id.id()]
                        .buffs[ACTOR_BUFF_SHRINK]
                        .Active()) {
                    if (pActors[attacker_id.id()]
                            .buffs[ACTOR_BUFF_SHRINK]
                            .power > 0)
                        dmgToRecv =
                            dmgToRecv / pActors[attacker_id.id()]
                                            .buffs[ACTOR_BUFF_SHRINK]
                                            .power;
                }
                if (pActors[actor_id].buffs[ACTOR_BUFF_STONED].Active())
                    dmgToRecv = 0;
                if (a4 == ABILITY_ATTACK1) {
                    v12 =
                        pActors[attacker_id.id()].monsterInfo.attack1Type;
                } else if (a4 == ABILITY_ATTACK2) {
                    v12 =
                        pActors[attacker_id.id()].monsterInfo.attack2Type;
                    if (pActors[actor_id]
                            .buffs[ACTOR_BUFF_SHIELD]
                            .Active())
                        dmgToRecv = dmgToRecv / 2;
                } else if (a4 == ABILITY_SPELL1) {
                    v12 = pSpellStats
                        ->pInfos[pActors[attacker_id.id()].monsterInfo.spell1Id]
                        .damageType;
                } else if (a4 == ABILITY_SPELL2) {
                    v12 = pSpellStats
                        ->pInfos[pActors[attacker_id.id()].monsterInfo.spell2Id]
                        .damageType;
                } else if (a4 == ABILITY_SPECIAL) {
                    v12 = static_cast<DamageType>(pActors[attacker_id.id()].monsterInfo.field_3C_some_special_attack);
                } else {
                    v12 = DAMAGE_PHYSICAL;
                }
                finalDmg = pActors[actor_id].CalcMagicalDamageToActor(
                    v12, dmgToRecv);
                pActors[actor_id].hp -= finalDmg;
                if (finalDmg) {
                    if (pActors[actor_id].hp > 0)
                        Actor::AI_Stun(actor_id, attacker_id, 0);
                    else
                        Actor::Die(actor_id);
                    Actor::AggroSurroundingPeasants(actor_id, 0);
                    pushDistance =
                        20 * finalDmg / pActors[actor_id].monsterInfo.hp;
                    if (pushDistance > 10) pushDistance = 10;
                    if (supertypeForMonsterId(pActors[actor_id].monsterInfo.id) != MONSTER_SUPERTYPE_TREANT) {
                        pActors[actor_id].velocity = 50 * pushDistance * pVelocity;
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
    MonsterId summonMonsterBaseType;         // esi@1
    int v5;                            // edx@2
    int v7;                            // edi@10
    MonsterInfo *v9;                   // ebx@10
    // MonsterDesc *v10; // edi@10
    int v13;                 // ebx@10
    int64_t v15;                 // edi@10
    int64_t v17;                 // ebx@10
    MonsterType v19;        // qax@10
    int v27;                 // [sp+18h] [bp-10h]@10
    int actorSector;         // [sp+1Ch] [bp-Ch]@8

    actorSector = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        actorSector = pIndoor->GetSector(this->pos);

    v19 = this->ally;
    if (this->ally == MONSTER_TYPE_INVALID) {
        v19 = monsterTypeForMonsterId(this->monsterInfo.id); // Original binary had an off by one here, forgetting the last +1.
    }
    v27 = uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ? 128 : 64;
    v13 = grng->random(2048);
    v15 = TrigLUT.cos(v13) * v27 + this->pos.x;
    v17 = TrigLUT.sin(v13) * v27 + this->pos.y;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        int sectorId = pIndoor->GetSector(v15, v17, this->pos.z);
        if (sectorId != actorSector) return;
        int z = BLV_GetFloorLevel(Vec3f(v15, v17, v27), sectorId);
        if (z == -30000) return;
        if (std::abs(z - v27) > 1024) return;
    }

    extraSummonLevel = this->monsterInfo.specialAbilityDamageDiceRolls;
    // TODO(captainurist): drop the cast here, store the data properly.
    summonMonsterBaseType = static_cast<MonsterId>(this->monsterInfo.field_3C_some_special_attack);
    if (extraSummonLevel) {
        if (extraSummonLevel >= 1 && extraSummonLevel <= 3) {
            // TODO(captainurist): encapsulate monster level arithmetic properly.
            summonMonsterBaseType = static_cast<MonsterId>(std::to_underlying(summonMonsterBaseType) + extraSummonLevel - 1);
        }
    } else {
        v5 = grng->random(100);
        // TODO(captainurist): encapsulate monster level arithmetic properly.
        if (v5 >= 90)
            summonMonsterBaseType = static_cast<MonsterId>(std::to_underlying(summonMonsterBaseType) + 2);
        else if (v5 >= 60)
            summonMonsterBaseType = static_cast<MonsterId>(std::to_underlying(summonMonsterBaseType) + 1);
    }
    Actor *actor = AllocateActor(true);
    if (!actor)
        return;

    v9 = &pMonsterStats->infos[summonMonsterBaseType];
    actor->name = v9->name;
    actor->hp = v9->hp;
    actor->monsterInfo = *v9;
    actor->monsterId = summonMonsterBaseType;
    actor->radius = pMonsterList->monsters[summonMonsterBaseType].monsterRadius;
    actor->height = pMonsterList->monsters[summonMonsterBaseType].monsterHeight;
    actor->monsterInfo.goldDiceRolls = 0;
    actor->monsterInfo.treasureType = RANDOM_ITEM_ANY;
    actor->monsterInfo.exp = 0;
    actor->moveSpeed = pMonsterList->monsters[summonMonsterBaseType].movementSpeed;

    actor->initialPosition.x = v15;
    actor->initialPosition.y = v17;
    actor->initialPosition.z = this->pos.z;
    actor->pos.x = v15;
    actor->pos.y = v17;
    actor->pos.z = this->pos.z;

    actor->tetherDistance = 256;
    actor->sectorId = actorSector;
    actor->PrepareSprites(0);
    actor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
    actor->ally = v19;
    actor->currentActionTime = 0_ticks;
    actor->group = this->group;
    actor->aiState = Summoned;
    actor->currentActionLength = 256_ticks;
    actor->UpdateAnimation();

    ++this->monsterInfo.specialAbilityDamageDiceBonus;
    if (ActorEnemy())
        actor->attributes |= ACTOR_AGGRESSOR;
    actor->summonerId = Pid(OBJECT_Actor, summonerId);
}

//----- (00401A91) --------------------------------------------------------
void Actor::UpdateActorAI() {
    double v42;              // st7@176
    double v43;              // st6@176
    ActorAbility v45;                 // eax@192
    SpellId v46;     // cl@197
    Duration v47;          // st7@206
    Duration v58;                // st7@246
    unsigned int v65{};        // [sp-10h] [bp-C0h]@144
    int v70;                 // [sp-10h] [bp-C0h]@213
    AIDirection v72;         // [sp+0h] [bp-B0h]@246
    ObjectType target_pid_type;     // [sp+70h] [bp-40h]@83
    AIDirection *pDir;       // [sp+7Ch] [bp-34h]@129
    int v81;                 // [sp+9Ch] [bp-14h]@100
    Pid target_pid;   // [sp+ACh] [bp-4h]@83
    unsigned v38;

    // Build AI array
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        Actor::MakeActorAIList_ODM();
    else
        Actor::MakeActorAIList_BLV();

    // Armageddon damage mechanic
    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR && pParty->armageddon_timer)
        armageddonProgress();

    // Turn-based mode: return
    if (pParty->bTurnBasedModeOn) {
        pTurnEngine->AITurnBasedAction();
        return;
    }

    // this loops over all actors in background ai state
    for (unsigned i = 0; i < pActors.size(); ++i) {
        Actor *pActor = &pActors[i];
        ai_near_actors_targets_pid[i] = Pid(OBJECT_Character, 0);

        // Skip actor if: Dead / Removed / Disabled / or in full ai state
        if (pActor->aiState == Dead || pActor->aiState == Removed ||
            pActor->aiState == Disabled || pActor->attributes & ACTOR_FULL_AI_STATE)
            continue;

        // Kill actor if HP == 0
        if (!pActor->hp && pActor->aiState != Dying)
            Actor::Die(i);

        // Kill buffs if expired
        for (ActorBuff i : pActor->buffs.indices())
            pActor->buffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime());

        // If shrink expired: reset height
        if (pActor->buffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->height = pMonsterList->monsters[pActor->monsterInfo.id].monsterHeight;
            pActor->buffs[ACTOR_BUFF_SHRINK].Reset();
        }

        // If Charm still active: make actor friendly
        if (pActor->buffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
        } else if (pActor->buffs[ACTOR_BUFF_CHARM].Expired()) {
          // Else: reset hostilty
          pActor->monsterInfo.hostilityType = pMonsterStats->infos[pActor->monsterInfo.id].hostilityType;
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
        pActor->monsterInfo.recoveryTime = std::max(pActor->monsterInfo.recoveryTime - pEventTimer->dt(), 0_ticks); // was pMiscTimer

        pActor->currentActionTime += pEventTimer->dt(); // was pMiscTimer
        if (pActor->currentActionTime < pActor->currentActionLength)
            continue;

        if (pActor->aiState == Dying) {
            pActor->aiState = Dead;
        } else {
            if (pActor->aiState != Summoned) {
                Actor::AI_StandOrBored(i, Pid(OBJECT_Character, 0), 256_ticks, nullptr);
                continue;
            }
            pActor->aiState = Standing;
        }

        pActor->currentActionTime = 0_ticks;
        pActor->currentActionLength = 0_ticks;
        pActor->UpdateAnimation();
    }

    // loops over for the actors in "full" ai state
    for (int v78 = 0; v78 < ai_arrays_size; ++v78) {
        unsigned actor_id = ai_near_actors_ids[v78];
        assert(actor_id < pActors.size());
        Pid actorPid = Pid(OBJECT_Actor, actor_id);

        Actor *pActor = &pActors[actor_id];

        v47 = pActor->monsterInfo.recoveryTime * flt_debugrecmod3;

        Actor::_SelectTarget(actor_id, &ai_near_actors_targets_pid[actor_id], true);

        if (pActor->monsterInfo.hostilityType != HOSTILITY_FRIENDLY && !ai_near_actors_targets_pid[actor_id])
            pActor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;

        target_pid = ai_near_actors_targets_pid[actor_id];
        target_pid_type = target_pid.type();

        float radiusMultiplier = target_pid_type == OBJECT_Actor ? 0.5 : 1.0;

        // v22 = pActor->uAIState;
        if (pActor->aiState == Dying || pActor->aiState == Dead || pActor->aiState == Removed ||
            pActor->aiState == Disabled || pActor->aiState == Summoned)
            continue;

        if (!pActor->hp)
            Actor::Die(actor_id);

        for (ActorBuff i : pActor->buffs.indices())
            pActor->buffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime());

        if (pActor->buffs[ACTOR_BUFF_SHRINK].Expired()) {
            pActor->height = pMonsterList->monsters[pActor->monsterInfo.id].monsterHeight;
            pActor->buffs[ACTOR_BUFF_SHRINK].Reset();
        }

        if (pActor->buffs[ACTOR_BUFF_CHARM].Active()) {
            pActor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
        } else if (pActor->buffs[ACTOR_BUFF_CHARM].Expired()) {
            pActor->monsterInfo.hostilityType = pMonsterStats->infos[pActor->monsterInfo.id].hostilityType;
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

        pActor->monsterInfo.recoveryTime = std::max(0_ticks, pActor->monsterInfo.recoveryTime - pEventTimer->dt()); // was pMiscTimer
        pActor->currentActionTime += pEventTimer->dt(); // was pMiscTimer

        if (!pActor->ActorNearby())
            pActor->attributes |= ACTOR_NEARBY;

        AIDirection targetDirection;
        Actor::GetDirectionInfo(actorPid, target_pid, &targetDirection, 0);
        pDir = &targetDirection;
        AIState uAIState = pActor->aiState;

         // TODO(captainurist): this check makes no sense, it fails only for monsters that are:
        // stunned && non-friendly && recovering && far from target && don't have missile attack. Seriously?
        if (pActor->monsterInfo.hostilityType == HOSTILITY_FRIENDLY ||
            pActor->monsterInfo.recoveryTime > 0_ticks ||
            radiusMultiplier * meleeRange < pDir->uDistance ||
            uAIState != Pursuing && uAIState != Standing && uAIState != Tethered && uAIState != Fidgeting && pActor->monsterInfo.attack1MissileType == MONSTER_PROJECTILE_NONE ||
            uAIState != Stunned) {
            if (pActor->currentActionTime < pActor->currentActionLength) {
                continue;
            } else if (pActor->aiState == AttackingMelee) {
                pushMeleeAttack(actorPid, pActor->pos + Vec3f(0, 0, pActor->height / 2), pActor->special_ability_use_check(actor_id));
            } else if (pActor->aiState == AttackingRanged1) {
                Actor::AI_RangedAttack(actor_id, pDir, pActor->monsterInfo.attack1MissileType, ABILITY_ATTACK1);  // light missile
            } else if (pActor->aiState == AttackingRanged2) {
                Actor::AI_RangedAttack(actor_id, pDir, pActor->monsterInfo.attack2MissileType, ABILITY_ATTACK2);  // arrow
            } else if (pActor->aiState == AttackingRanged3) {
                Actor::AI_SpellAttack(actor_id, pDir, pActor->monsterInfo.spell1Id, ABILITY_SPELL1,
                                      pActor->monsterInfo.spell1SkillMastery);
            } else if (pActor->aiState == AttackingRanged4) {
                Actor::AI_SpellAttack(actor_id, pDir, pActor->monsterInfo.spell2Id, ABILITY_SPELL2,
                                      pActor->monsterInfo.spell2SkillMastery);
            }
        }

        int distanceToTarget = pDir->uDistance;

        MonsterHostility relationToTarget;
        if (pActor->monsterInfo.hostilityType == HOSTILITY_FRIENDLY) {
            if (target_pid_type == OBJECT_Actor) {
                relationToTarget = pFactionTable->relations[monsterTypeForMonsterId(pActor->monsterInfo.id)]
                                                           [monsterTypeForMonsterId(pActors[target_pid.id()].monsterInfo.id)];
            } else {
                relationToTarget = HOSTILITY_LONG;
            }
            v38 = 0;
            if (relationToTarget == HOSTILITY_SHORT)
                v38 = 1024;
            else if (relationToTarget == HOSTILITY_MEDIUM)
                v38 = 2560;
            else if (relationToTarget == HOSTILITY_LONG)
                v38 = 5120;
            if (relationToTarget >= HOSTILITY_CLOSE && relationToTarget <= HOSTILITY_LONG && distanceToTarget < v38 || relationToTarget == HOSTILITY_CLOSE)
                pActor->monsterInfo.hostilityType = HOSTILITY_LONG;
        }

        // If actor afraid: flee or if out of range random move
        if (pActor->buffs[ACTOR_BUFF_AFRAID].Active()) {
            if (distanceToTarget >= 10240)
                Actor::AI_RandomMove(actor_id, target_pid, 1024, 0_ticks);
            else
                Actor::AI_Flee(actor_id, target_pid, 0_ticks, pDir);
            continue;
        }

        if (pActor->monsterInfo.hostilityType == HOSTILITY_LONG &&
            target_pid) {
            if (pActor->monsterInfo.aiType == MONSTER_AI_WIMP) {
                if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                    Actor::AI_Stand(actor_id, target_pid, pActor->monsterInfo.recoveryTime * flt_debugrecmod3, pDir);
                } else {
                    Actor::AI_Flee(actor_id, target_pid, 0_ticks, pDir);
                    continue;
                }
            }
            if (!(pActor->attributes & ACTOR_FLEEING)) {
                if (pActor->monsterInfo.aiType == MONSTER_AI_NORMAL || pActor->monsterInfo.aiType == MONSTER_AI_AGGRESSIVE) {
                    if (pActor->monsterInfo.aiType == MONSTER_AI_NORMAL)
                        v43 = pActor->monsterInfo.hp * 0.2;
                    if (pActor->monsterInfo.aiType == MONSTER_AI_AGGRESSIVE)
                        v43 = pActor->monsterInfo.hp * 0.1;
                    v42 = pActor->hp;
                    if (v43 > v42 && distanceToTarget < 10240) {
                        Actor::AI_Flee(actor_id, target_pid, 0_ticks, pDir);
                        continue;
                    }
                }
            }

            v81 = distanceToTarget - pActor->radius;
            if (target_pid_type == OBJECT_Actor)
                v81 -= pActors[target_pid.id()].radius;
            if (v81 < 0)
                v81 = 0;
            // rand();
            pActor->attributes &= ~ACTOR_LAST_SPELL_MISSED;  // ~0x40000
            if (v81 < 5120) {
                v45 = pActor->special_ability_use_check(actor_id);
                if (v45 == ABILITY_ATTACK1) {
                    if (pActor->monsterInfo.attack1MissileType != MONSTER_PROJECTILE_NONE) {
                        if (pActor->monsterInfo.recoveryTime <= 0_ticks) {
                            Actor::AI_MissileAttack1(actor_id, target_pid, pDir);
                        } else if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            if (radiusMultiplier * meleeRange > v81)
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            else
                                Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
                        }
                    } else {
                        if (v81 >= radiusMultiplier * meleeRange) {
                            if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            } else if (v81 >= 1024) {  // monsters
                                Actor::AI_Pursue3(actor_id, target_pid, 0_ticks, pDir);
                            } else {
                                v70 = (radiusMultiplier * meleeRange);
                                // monsters
                                // guard after player runs away
                                // follow player
                                Actor::AI_Pursue2(actor_id, target_pid, 0_ticks, pDir, v70);
                            }
                        } else if (pActor->monsterInfo.recoveryTime > 0_ticks) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            // monsters
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                } else if (v45 == ABILITY_SPELL1 || v45 == ABILITY_SPELL2) {
                    if (v45 == ABILITY_SPELL1)
                        v46 = pActor->monsterInfo.spell1Id;
                    else
                        v46 = pActor->monsterInfo.spell2Id;
                    if (v46 != SPELL_NONE) {
                        if (pActor->monsterInfo.recoveryTime <= 0_ticks) {
                            if (v45 == ABILITY_SPELL1)
                                Actor::AI_SpellAttack1(actor_id, target_pid, pDir);
                            else
                                Actor::AI_SpellAttack2(actor_id, target_pid, pDir);
                        } else if (radiusMultiplier * meleeRange > v81 || pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
                        }
                    } else {
                        // v45 == ABILITY_ATTACK2
                        if (v81 >= radiusMultiplier * meleeRange) {
                            if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                            } else if (v81 >= 1024) {
                                Actor::AI_Pursue3(actor_id, target_pid, 256_ticks, pDir);
                            } else {
                                v70 = (radiusMultiplier * meleeRange);
                                Actor::AI_Pursue2(actor_id, target_pid, 0_ticks, pDir, v70);
                            }
                        } else if (pActor->monsterInfo.recoveryTime > 0_ticks) {
                            Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                        } else {
                            Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
                        }
                    }
                    continue;
                }
            }
        }

        if (pActor->monsterInfo.hostilityType != HOSTILITY_LONG ||
            !target_pid || v81 >= 5120 || v45 != ABILITY_ATTACK2) {
            if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_SHORT) {
                Actor::AI_RandomMove(actor_id, Pid::character(0), 1024, 0_ticks);
            } else if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_MEDIUM) {
                Actor::AI_RandomMove(actor_id, Pid::character(0), 2560, 0_ticks);
            } else if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_LONG) {
                Actor::AI_RandomMove(actor_id, Pid::character(0), 5120, 0_ticks);
            } else if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_FREE) {
                Actor::AI_RandomMove(actor_id, Pid::character(0), 10240, 0_ticks);
            } else if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                Actor::GetDirectionInfo(actorPid, Pid::character(0), &v72, 0);
                v58 = pActor->monsterInfo.recoveryTime * flt_debugrecmod3;
                Actor::AI_Stand(actor_id, Pid::character(0), v58, &v72);
            }
        } else if (pActor->monsterInfo.attack2MissileType == MONSTER_PROJECTILE_NONE) {
            if (v81 >= radiusMultiplier * meleeRange) {
                if (pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY) {
                    Actor::AI_Stand(actor_id, target_pid, v47, pDir);
                } else if (v81 >= 1024) {
                    Actor::AI_Pursue3(actor_id, target_pid, 256_ticks, pDir);
                } else {
                    v70 = (radiusMultiplier * meleeRange);
                    Actor::AI_Pursue2(actor_id, target_pid, 0_ticks, pDir, v70);
                }
            } else if (pActor->monsterInfo.recoveryTime > 0_ticks) {
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            } else {
                Actor::AI_MeleeAttack(actor_id, target_pid, pDir);
            }
        } else if (pActor->monsterInfo.recoveryTime > 0_ticks) {
            if (radiusMultiplier * meleeRange > v81 || pActor->monsterInfo.movementType == MONSTER_MOVEMENT_TYPE_STATIONARY)
                Actor::AI_Stand(actor_id, target_pid, v47, pDir);
            else
                Actor::AI_Pursue1(actor_id, target_pid, actor_id, v47, pDir);
        } else {
            Actor::AI_MissileAttack2(actor_id, target_pid, pDir);
        }
    }
}

bool Actor::isActorKilled(ActorKillCheckPolicy policy, int param, int count) {
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
        deadActors = Actor::searchDeadActorsByMonsterID(&totalActors, static_cast<MonsterId>(param));
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

    for (unsigned i = 0; i < pActors.size(); i++) {
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

int Actor::searchDeadActorsByMonsterID(int *pTotalActors, MonsterId monsterID) {
    int result = 0, totalActors = 0;
    bool alert = GetAlertStatus();

    for (unsigned i = 0; i < pActors.size(); i++) {
        if (!!(pActors[i].attributes & ACTOR_UNKNOW7) == alert && pActors[i].monsterInfo.id == monsterID) {
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

    for (unsigned i = 0; i < pActors.size(); i++) {
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
    if (engine->_currentLoadedMapId == MAP_CELESTE) {
        bCelestia = true;
    }
    if (engine->_currentLoadedMapId == MAP_PIT) {
        bPit = true;
    }
    if (pParty->isPartyGood())
        good = true;
    if (pParty->isPartyEvil())
        evil = true;

    std::ranges::fill(ai_near_actors_targets_pid, Pid());

    for (unsigned i = 0; i < pActors.size(); ++i) {
        Actor *actor = &pActors[i];

        if (actor->CanAct() || actor->aiState == Disabled) {
            actor->pos.x = actor->initialPosition.x;
            actor->pos.y = actor->initialPosition.y;
            actor->pos.z = actor->initialPosition.z;
            actor->hp = actor->monsterInfo.hp;
            if (actor->aiState != Disabled) {
                Actor::AI_Stand(i, ai_near_actors_targets_pid[i],
                                actor->monsterInfo.recoveryTime, 0);
            }
        }

        actor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;

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
int Actor::DamageMonsterFromParty(Pid a1, unsigned int uActorID_Monster, const Vec3f &pVelocity) {
    SpriteObject *projectileSprite;  // ebx@1
    Actor *pMonster;                 // esi@7
    Duration extraRecoveryTime;           // qax@125
    uint16_t v43{};            // ax@132
    uint16_t v45{};            // ax@132
    // uint64_t v46; // [sp+Ch] [bp-60h]@6
    int skillLevel = 0;                    // [sp+44h] [bp-28h]@1
    bool IsAdditionalDamagePossible;  // [sp+50h] [bp-1Ch]@1
    int v61;                          // [sp+58h] [bp-14h]@1
    bool isLifeStealing;              // [sp+5Ch] [bp-10h]@1
    int uDamageAmount;                // [sp+60h] [bp-Ch]@1
    DamageType attackElement;        // [sp+64h] [bp-8h]@27

    projectileSprite = 0;
    uDamageAmount = 0;
    v61 = 0;
    IsAdditionalDamagePossible = false;
    isLifeStealing = 0;
    if (a1.type() == OBJECT_Sprite) {
        projectileSprite = &pSpriteObjects[a1.id()];
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        a1 = projectileSprite->spell_caster_pid;
    }
    if (a1.type() != OBJECT_Character) return 0;

    assert(a1.id() < 4);
    Character *character = &pParty->pCharacters[a1.id()];
    pMonster = &pActors[uActorID_Monster];
    if (pMonster->IsNotAlive()) return 0;

    pMonster->attributes |= ACTOR_NEARBY | ACTOR_ACTIVE;
    if (pMonster->aiState == Fleeing) pMonster->attributes |= ACTOR_FLEEING;
    bool hit_will_stun = false, hit_will_paralyze = false;
    if (!projectileSprite) {
        IsAdditionalDamagePossible = true;
        if (InventoryEntry mainHandItem = character->inventory.functionalEntry(ITEM_SLOT_MAIN_HAND)) {
            Skill main_hand_skill = mainHandItem->skill();
            Mastery main_hand_mastery = character->getSkillValue(main_hand_skill).mastery();
            switch (main_hand_skill) {
                case SKILL_STAFF:
                    if (main_hand_mastery >= MASTERY_MASTER) {
                        if (grng->random(100) < character->getActualSkillValue(SKILL_STAFF).level())
                            hit_will_stun = true;
                    }
                    break;

                case SKILL_MACE:
                    if (main_hand_mastery >= MASTERY_MASTER) {
                        if (grng->random(100) < character->getActualSkillValue(SKILL_MACE).level())
                            hit_will_stun = true;
                    }
                    if (main_hand_mastery >= MASTERY_GRANDMASTER) {
                        if (grng->random(100) < character->getActualSkillValue(SKILL_MACE).level())
                            hit_will_paralyze = true;
                    }
                    break;

                default:
                    break;
            }
        }
        attackElement = DAMAGE_PHYSICAL;
        uDamageAmount = character->CalculateMeleeDamageTo(false, false, pMonster->monsterInfo.id);
        if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
            character->playReaction(SPEECH_ATTACK_MISS);
            return 0;
        }
    } else {
        v61 = projectileSprite->field_60_distance_related_prolly_lod;
        if (projectileSprite->uSpellID != SPELL_DARK_SOULDRINKER) {
            int d1 = std::abs(pParty->pos.x - projectileSprite->vPosition.x);
            int d2 = std::abs(pParty->pos.y - projectileSprite->vPosition.y);
            int d3 = std::abs(pParty->pos.z - projectileSprite->vPosition.z);
            v61 = int_get_vector_length(d1, d2, d3);

            if (v61 >= 5120 && !(pMonster->attributes & ACTOR_FULL_AI_STATE))  // 0x400
                return 0;
            else if (v61 >= 2560)
                v61 = 2;
            else
                v61 = 1;
        }

        switch (projectileSprite->uSpellID) {
            case SPELL_LASER_PROJECTILE:
                // TODO: should be changed to GetActual* equivalents?
                v61 = 1;
                if (character->getSkillValue(SKILL_BLASTER).mastery() >= MASTERY_MASTER)
                    skillLevel = character->getSkillValue(SKILL_BLASTER).level();
                attackElement = DAMAGE_PHYSICAL;
                uDamageAmount = character->CalculateMeleeDamageTo(true, true, MONSTER_INVALID);
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return 0;
                }
                break;
            case SPELL_101:
                attackElement = DAMAGE_FIRE;
                uDamageAmount = character->CalculateRangedDamageTo(MONSTER_INVALID);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = true;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return 0;
                }
                break;
            case SPELL_EARTH_BLADES:
                skillLevel = 5 * projectileSprite->spell_level;
                attackElement = pSpellStats->pInfos[SPELL_EARTH_BLADES].damageType;
                uDamageAmount = CalcSpellDamage(
                    SPELL_EARTH_BLADES, projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->hp);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount >>= 1;
                IsAdditionalDamagePossible = false;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return 0;
                }
                break;
            case SPELL_EARTH_STUN:
                uDamageAmount = 0;
                attackElement = DAMAGE_PHYSICAL;
                hit_will_stun = 1;
                if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return 0;
                }
                break;
            case SPELL_BOW_ARROW:
                attackElement = DAMAGE_PHYSICAL;
                uDamageAmount = character->CalculateRangedDamageTo(
                    pMonster->monsterId);
                if (pMonster->buffs[ACTOR_BUFF_SHIELD].Active())
                    uDamageAmount /= 2;
                IsAdditionalDamagePossible = true;
                if (projectileSprite->containing_item.itemId != ITEM_NULL &&
                    projectileSprite->containing_item.specialEnchantment == ITEM_ENCHANTMENT_OF_CARNAGE) {
                    attackElement = DAMAGE_FIRE;
                } else if (!character->characterHitOrMiss(pMonster, v61, skillLevel)) {
                    character->playReaction(SPEECH_ATTACK_MISS);
                    return 0;
                }
                break;

            default:
                attackElement = pSpellStats->pInfos[projectileSprite->uSpellID].damageType;
                IsAdditionalDamagePossible = false;
                uDamageAmount = CalcSpellDamage(
                    projectileSprite->uSpellID,
                    projectileSprite->spell_level,
                    projectileSprite->spell_skill, pMonster->hp);
                break;
        }
    }

    if (character->IsWeak()) uDamageAmount /= 2;
    if (pMonster->buffs[ACTOR_BUFF_STONED].Active()) uDamageAmount = 0;
    v61 = pMonster->CalcMagicalDamageToActor(attackElement, uDamageAmount);
    if (!projectileSprite && character->IsUnarmed() &&
        character->pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].Active()) {
        v61 += pMonster->CalcMagicalDamageToActor(
            DAMAGE_BODY,
            character->pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].power);
    }
    uDamageAmount = v61;
    if (IsAdditionalDamagePossible) {
        if (projectileSprite) {
            skillLevel =
                projectileSprite->containing_item._439DF3_get_additional_damage(
                    &attackElement, &isLifeStealing);
            if (isLifeStealing && pMonster->hp > 0) {
                character->health += v61 / 5;
                if (character->health > character->GetMaxHealth())
                    character->health = character->GetMaxHealth();
            }
            uDamageAmount +=
                pMonster->CalcMagicalDamageToActor(attackElement, skillLevel);
        } else {
            for (ItemSlot i : {ITEM_SLOT_OFF_HAND, ITEM_SLOT_MAIN_HAND}) {
                if (InventoryEntry item = character->inventory.functionalEntry(i)) {
                    skillLevel = item->_439DF3_get_additional_damage(&attackElement,
                                                             &isLifeStealing);
                    if (isLifeStealing && pMonster->hp > 0) {
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
    pMonster->hp -= uDamageAmount;
    if (uDamageAmount == 0 && !hit_will_stun) {
        character->playReaction(SPEECH_ATTACK_MISS);
        return 0;
    }
    if (pMonster->hp > 0) {
        Actor::AI_Stun(uActorID_Monster, a1, 0);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (engine->config->settings.ShowHits.value()) {
            if (projectileSprite)
                engine->_statusBar->setEvent(LSTR_S_SHOOTS_S_FOR_LU_POINTS, character->name, pMonster->name, uDamageAmount);
            else
                engine->_statusBar->setEvent(LSTR_S_HITS_S_FOR_LU_DAMAGE, character->name, pMonster->name, uDamageAmount);
        }
    } else {
        Actor::Die(uActorID_Monster);
        Actor::ApplyFineForKillingPeasant(uActorID_Monster);
        Actor::AggroSurroundingPeasants(uActorID_Monster, 1);
        if (pMonster->monsterInfo.exp)
            pParty->GivePartyExp(
                pMonsterStats->infos[pMonster->monsterInfo.id].exp);
        SpeechId speech = SPEECH_ATTACK_HIT;
        if (vrng->random(100) < 20) {
            speech = pMonster->monsterInfo.hp >= 100 ? SPEECH_KILL_STRONG_ENEMY : SPEECH_KILL_WEAK_ENEMY;
        }
        character->playReaction(speech);
        if (engine->config->settings.ShowHits.value()) {
            engine->_statusBar->setEvent(LSTR_S_INFLICTS_LU_POINTS_KILLING_S, character->name, uDamageAmount, pMonster->name);
        }
    }
    if (pMonster->buffs[ACTOR_BUFF_PAIN_REFLECTION].Active() && uDamageAmount != 0)
        character->receiveDamage(uDamageAmount, attackElement);
    int knockbackValue = 20 * v61 / (signed int)pMonster->monsterInfo.hp;
    if ((character->GetSpecialItemBonus(ITEM_ENCHANTMENT_OF_FORCE) ||
         hit_will_stun) && pMonster->DoesDmgTypeDoDamage(DAMAGE_EARTH)) {
        extraRecoveryTime = 20_ticks;
        knockbackValue = 10;
        if (!pParty->bTurnBasedModeOn)
            extraRecoveryTime = debug_combat_recovery_mul * flt_debugrecmod3 * 20_ticks;
        pMonster->monsterInfo.recoveryTime += extraRecoveryTime;
        if (engine->config->settings.ShowHits.value()) {
            engine->_statusBar->setEvent(LSTR_S_STUNS_S, character->name, pMonster->name);
        }
    }
    if (hit_will_paralyze && pMonster->CanAct() &&
        pMonster->DoesDmgTypeDoDamage(DAMAGE_EARTH)) {
        CombinedSkillValue maceSkill = character->getActualSkillValue(SKILL_MACE);
        pMonster->buffs[ACTOR_BUFF_PARALYZED].Apply(pParty->GetPlayingTime() + Duration::fromMinutes(maceSkill.level()), maceSkill.mastery(), 0, 0, 0);
        if (engine->config->settings.ShowHits.value()) {
            engine->_statusBar->setEvent(LSTR_S_PARALYZES_S, character->name, pMonster->name);
        }
    }
    if (knockbackValue > 10) knockbackValue = 10;
    if (supertypeForMonsterId(pMonster->monsterInfo.id) != MONSTER_SUPERTYPE_TREANT) {
        pMonster->velocity = 50 * knockbackValue * pVelocity;
    }
    Actor::AddOnDamageOverlay(uActorID_Monster, 1, v61);

    return uDamageAmount;
}

//----- (004BBF61) --------------------------------------------------------
void Actor::Arena_summon_actor(MonsterId monster_id, Vec3f pos) {
    Actor *actor = AllocateActor(true);
    if (!actor)
        return;

    int v16 = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        v16 = pIndoor->GetSector(pos);

    actor->name = pMonsterStats->infos[monster_id].name;
    actor->hp = (short)pMonsterStats->infos[monster_id].hp;
    actor->monsterInfo = pMonsterStats->infos[monster_id];
    actor->monsterId = monster_id;
    actor->radius = pMonsterList->monsters[monster_id].monsterRadius;
    actor->height = pMonsterList->monsters[monster_id].monsterHeight;
    actor->moveSpeed = pMonsterList->monsters[monster_id].movementSpeed;
    actor->initialPosition = pos;
    actor->pos = pos;
    actor->attributes |= ACTOR_AGGRESSOR;
    actor->monsterInfo.treasureType = RANDOM_ITEM_ANY;
    actor->monsterInfo.treasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    actor->monsterInfo.goldDiceSides = 0;
    actor->monsterInfo.goldDiceRolls = 0;
    actor->monsterInfo.treasureDropChance = 0;
    actor->tetherDistance = 256;
    actor->sectorId = v16;
    actor->group = 1;
    actor->monsterInfo.hostilityType = HOSTILITY_LONG;
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
    std::array<int, 60> victims;
    signed int victimCount = 0;
    if (pActor->monsterInfo.attackPreferences) {
        for (MonsterAttackPreference preference : allMonsterAttackPreferences()) {
            if (pActor->monsterInfo.attackPreferences & preference) {
                for (int i = 0; i < pParty->pCharacters.size(); ++i) {
                    if (!pParty->pCharacters[i].matchesAttackPreference(preference))
                        continue;

                    if (pParty->pCharacters[i].conditions.hasNone({CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD,
                                                                   CONDITION_PETRIFIED, CONDITION_ERADICATED}))
                        victims[victimCount++] = i;
                }
            }
        }
        if (victimCount)
            return victims[grng->random(victimCount)];
    }
    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        if (pParty->pCharacters[i].conditions.hasNone({CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD,
                                                       CONDITION_PETRIFIED, CONDITION_ERADICATED}))
            victims[victimCount++] = i;
    }
    if (victimCount)
        return victims[grng->random(victimCount)];
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
Pid stru319::FindClosestActor(int pick_depth, int a3 /*Relates to targeting/not targeting allies?*/, int target_undead) {
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
        return EngineIocContainer::ResolveVis()->PickClosestActor(OBJECT_Actor, pick_depth, static_cast<VisSelectFlags>(select_flags), 657456, -1);
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
    if (v11.type() == OBJECT_Actor)
    {
    if ( pActors[v11.id()].uAIState != Dead )
    {
    if ( pActors[v11.id()].uAIState != Dying && pActors[v11.id()].uAIState
    != Removed
    && pActors[v11.id()].uAIState != Summoned &&
    pActors[v11.id()].uAIState != Disabled
    && (!a3 || pActors[v11.id()].GetActorsRelation(0)) )
    {
    if ( (!a4 ||
    MonsterStats::BelongsToSupertype(pActors[v11.id()].pMonsterInfo.uID,
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

    for (unsigned i = 0; i < pActors.size(); ++i) {
        for_x = std::abs(pActors[i].pos.x - pParty->pos.x);
        for_y = std::abs(pActors[i].pos.y - pParty->pos.y);
        for_z = std::abs(pActors[i].pos.z - pParty->pos.z);
        if (int_get_vector_length(for_x, for_y, for_z) < distance) {
            if (pActors[i].aiState != Dead) {
                if (pActors[i].aiState != Dying &&
                    pActors[i].aiState != Removed &&
                    pActors[i].aiState != Disabled &&
                    pActors[i].aiState != Summoned &&
                    (pActors[i].ActorEnemy() ||
                     pActors[i].GetActorsRelation(0) != HOSTILITY_FRIENDLY))
                    return true;
            }
        }
    }
    return false;
}


void StatusBarItemFound(int num_gold_found, std::string_view item_unidentified_name) {
    if (num_gold_found != 0) {
        engine->_statusBar->setEvent(LSTR_YOU_FOUND_D_GOLD_AND_AN_ITEM_S, num_gold_found, item_unidentified_name);
    } else {
        engine->_statusBar->setEvent(LSTR_YOU_FOUND_AN_ITEM_S, item_unidentified_name);
    }
}


//----- (00426A5A) --------------------------------------------------------
void Actor::LootActor() {
    Item Dst;         // [sp+Ch] [bp-2Ch]@1
    bool itemFound;      // [sp+30h] [bp-8h]@1

    pParty->placeHeldItemInInventoryOrDrop();
    Dst.Reset();
    itemFound = false;
    int foundGold = 0;
    if (!ActorHasItem()) {
        foundGold = grng->randomDice(this->monsterInfo.goldDiceRolls, this->monsterInfo.goldDiceSides);
        if (foundGold) {
            pParty->partyFindsGold(foundGold, GOLD_RECEIVE_SHARE);
        }
    } else {
        if (this->items[3].isGold()) {
            foundGold = this->items[3].goldAmount;
            this->items[3].Reset();
            if (foundGold) {
                pParty->partyFindsGold(foundGold, GOLD_RECEIVE_SHARE);
            }
        }
    }
    if (this->carriedItemId != ITEM_NULL) {
        Dst.Reset();
        Dst.itemId = this->carriedItemId;
        Dst.postGenerate(ITEM_SOURCE_MONSTER);

        StatusBarItemFound(foundGold, pItemTable->items[Dst.itemId].unidentifiedName);

        if (!pParty->addItemToParty(&Dst)) {
            pParty->setHoldingItem(Dst);
        }
        this->carriedItemId = ITEM_NULL;
        if (this->items[0].itemId != ITEM_NULL) {
            if (!pParty->addItemToParty(&this->items[0])) {
                pParty->placeHeldItemInInventoryOrDrop();
                pParty->setHoldingItem(this->items[0]);
            }
            this->items[0].Reset();
        }
        if (this->items[1].itemId != ITEM_NULL) {
            if (!pParty->addItemToParty(&this->items[1])) {
                pParty->placeHeldItemInInventoryOrDrop();
                pParty->setHoldingItem(this->items[1]);
            }
            this->items[1].Reset();
        }
        this->Remove();
        return;
    }
    if (this->ActorHasItem()) {
        if (this->items[3].itemId != ITEM_NULL) {
            Dst = this->items[3];
            this->items[3].Reset();

            StatusBarItemFound(foundGold, pItemTable->items[Dst.itemId].unidentifiedName);

            if (!pParty->addItemToParty(&Dst)) {
                pParty->setHoldingItem(Dst);
            }
            itemFound = true;
        }
    } else {
        if (grng->random(100) < this->monsterInfo.treasureDropChance && this->monsterInfo.treasureLevel != ITEM_TREASURE_LEVEL_INVALID) {
            pItemTable->generateItem(this->monsterInfo.treasureLevel, this->monsterInfo.treasureType, &Dst);

            StatusBarItemFound(foundGold, pItemTable->items[Dst.itemId].unidentifiedName);

            if (!pParty->addItemToParty(&Dst)) {
                pParty->setHoldingItem(Dst);
            }
            itemFound = true;
        }
    }
    if (this->items[0].itemId != ITEM_NULL) {
        if (!pParty->addItemToParty(&this->items[0])) {
            pParty->placeHeldItemInInventoryOrDrop();
            pParty->setHoldingItem(this->items[0]);
            itemFound = true;
        }
        this->items[0].Reset();
    }
    if (this->items[1].itemId != ITEM_NULL) {
        if (!pParty->addItemToParty(&this->items[1])) {
            pParty->placeHeldItemInInventoryOrDrop();
            pParty->setHoldingItem(this->items[1]);
            itemFound = true;
        }
        this->items[1].Reset();
    }
    if (!itemFound || grng->random(100) < 90) {  // for repeatedly get gold and item
        this->Remove();
    }
}

//----- (00427102) --------------------------------------------------------
bool Actor::_427102_IsOkToCastSpell(SpellId spell) {
    switch (spell) {
        case SPELL_BODY_POWER_CURE: { // TODO(pskelton): Only cure below half health?
            if (this->hp >= this->monsterInfo.hp) return false;
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
        case SPELL_NONE:
            return false;
        default:
            return true;
    }
}

//----- (0042704B) --------------------------------------------------------
ActorAbility Actor::special_ability_use_check(int a2) {
    if (this->monsterInfo.specialAbilityType == MONSTER_SPECIAL_ABILITY_SUMMON && this->monsterInfo.specialAbilityDamageDiceBonus < 3 && grng->random(100) < 5)
        this->SummonMinion(a2);

    bool okToCastSpell1 = this->_427102_IsOkToCastSpell(this->monsterInfo.spell1Id);
    bool okToCastSpell2 = this->_427102_IsOkToCastSpell(this->monsterInfo.spell2Id);
    if (okToCastSpell1 && this->monsterInfo.spell1UseChance && grng->random(100) < this->monsterInfo.spell1UseChance)
        return ABILITY_SPELL1;
    if (okToCastSpell2 && this->monsterInfo.spell2UseChance && grng->random(100) < this->monsterInfo.spell2UseChance)
        return ABILITY_SPELL2;
    if (this->monsterInfo.attack2Chance && grng->random(100) < this->monsterInfo.attack2Chance)
        return ABILITY_ATTACK2;
    return ABILITY_ATTACK1;
}

//----- (004273BB) --------------------------------------------------------
bool Actor::_4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4) {
    signed int v6;   // ebx@1
    signed int v7;   // esi@1
    int armorSum;    // ebx@10
    signed int a2a;  // [sp+18h] [bp+Ch]@1

    v6 = defender->monsterInfo.ac;
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
    return grng->random(armorSum + 2 * this->monsterInfo.level + 10) + a2a + 1 >
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
    v4 = pPlayer->GetActualAC() + 2 * this->monsterInfo.level + 10;
    v5 = grng->random(v4) + 1;
    return (v3 + v5 > pPlayer->GetActualAC() + 5);
}

//----- (0042756B) --------------------------------------------------------
int Actor::CalcMagicalDamageToActor(DamageType dmgType,
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
        case DAMAGE_FIRE:
            v6 = this->monsterInfo.resFire;
            v4 = v5;
            break;
        case DAMAGE_AIR:
            v6 = this->monsterInfo.resAir;
            v4 = v5;
            break;
        case DAMAGE_WATER:
            v6 = this->monsterInfo.resWater;
            v4 = v5;
            break;
        case DAMAGE_EARTH:
            v6 = this->monsterInfo.resEarth;
            v4 = v5;
            break;
        case DAMAGE_PHYSICAL:
            v6 = this->monsterInfo.resPhysical;
            break;
        case DAMAGE_SPIRIT:
            v6 = this->monsterInfo.resSpirit;
            break;
        case DAMAGE_MIND:
            v6 = this->monsterInfo.resMind;
            v4 = v5;
            break;
        case DAMAGE_BODY:
            v6 = this->monsterInfo.resBody;
            v4 = v5;
            break;
        case DAMAGE_LIGHT:
            v6 = this->monsterInfo.resLight;
            break;
        case DAMAGE_DARK:
            v6 = this->monsterInfo.resDark;
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
bool Actor::DoesDmgTypeDoDamage(DamageType uType) {
    signed int resist;  // esi@2

    switch (uType) {
        case DAMAGE_FIRE:
            resist = this->monsterInfo.resFire;
            break;
        case DAMAGE_AIR:
            resist = this->monsterInfo.resAir;
            break;
        case DAMAGE_WATER:
            resist = this->monsterInfo.resWater;
            break;
        case DAMAGE_EARTH:
            resist = this->monsterInfo.resEarth;
            break;
        case DAMAGE_PHYSICAL:
            resist = this->monsterInfo.resPhysical;
            break;
        case DAMAGE_SPIRIT:
            resist = this->monsterInfo.resSpirit;
            break;
        case DAMAGE_MIND:
            resist = this->monsterInfo.resMind;
            break;
        case DAMAGE_BODY:
            resist = this->monsterInfo.resBody;
            break;
        case DAMAGE_LIGHT:
            resist = this->monsterInfo.resLight;
            break;
        case DAMAGE_DARK:
            resist = this->monsterInfo.resDark;
            break;
        default:
            return true;
    }
    if (resist < 200)
        return grng->random((this->monsterInfo.level >> 2) + resist + 30) < 30;
    else
        return false;
}

//----- (00448A98) --------------------------------------------------------
void toggleActorGroupFlag(unsigned int uGroupID, ActorAttribute uFlag,
                          bool bValue) {
    if (uGroupID) {
        if (bValue) {
            for (unsigned i = 0; i < (unsigned int)pActors.size(); ++i) {
                if (pActors[i].group == uGroupID) {
                    pActors[i].attributes |= uFlag;
                    if (uFlag == ACTOR_UNKNOW11) {
                        pActors[i].aiState = Disabled;
                        pActors[i].UpdateAnimation();
                    }
                }
            }
        } else {
            for (unsigned i = 0; i < (unsigned int)pActors.size(); ++i) {
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

    pParty->uFlags &= ~PARTY_FLAG_ALERT_RED_OR_YELLOW;

    for (Actor &actor : pActors) {
        actor.ResetFullAiState();
        if (!actor.CanAct()) {
            actor.ResetActive();
            continue;
        }

        int delta_x = std::abs(pParty->pos.x - actor.pos.x);
        int delta_y = std::abs(pParty->pos.y - actor.pos.y);
        int delta_z = std::abs(pParty->pos.z - actor.pos.z);

        int distance = int_get_vector_length(delta_x, delta_y, delta_z) - actor.radius;
        if (distance < 0)
            distance = 0;

        if (distance < 5632) {
            actor.ResetHostile();
            if (actor.ActorEnemy() || actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
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

    // and takes nearest amount
    int configLimit = engine->config->gameplay.MaxActiveAIActors.value();
    for (int i = 0; (i < configLimit) && (i < activeActorsDistances.size()); i++) {
        ai_near_actors_ids[i] = activeActorsDistances[i].first;
        pActors[ai_near_actors_ids[i]].attributes |= ACTOR_FULL_AI_STATE;
    }

    ai_arrays_size = std::min(configLimit, (int)activeActorsDistances.size());
}

//----- (004016FA) --------------------------------------------------------
int Actor::MakeActorAIList_BLV() {
    std::vector<std::pair<int, int>> activeActorsDistances; // pair<id, distance>
    std::vector<int> pickedActorIds;

    // reset party alert level
    pParty->uFlags &= ~PARTY_FLAG_ALERT_RED_OR_YELLOW;

    // find actors that are in range and can act
    for (Actor &actor : pActors) {
        actor.ResetFullAiState();
        if (!actor.CanAct()) {
            actor.ResetActive();
            continue;
        }

        int delta_x = std::abs(pParty->pos.x - actor.pos.x);
        int delta_y = std::abs(pParty->pos.y - actor.pos.y);
        int delta_z = std::abs(pParty->pos.z - actor.pos.z);

        int distance = int_get_vector_length(delta_x, delta_y, delta_z) - actor.radius;
        if (distance < 0)
            distance = 0;

        // actor is in range
        if (distance < 10240) {
            actor.ResetHostile();
            if (actor.ActorEnemy() || actor.GetActorsRelation(0) != HOSTILITY_FRIENDLY) {
                actor.attributes |= ACTOR_HOSTILE;
                if (!(pParty->GetRedAlert()) && (double)distance < meleeRange)
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
        if (pActors[actorId].ActorNearby() || Detect_Between_Objects(Pid(OBJECT_Actor, actorId), Pid(OBJECT_Character, 0))) {
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

    // activate ai state for first x actors from list
    int configLimit = engine->config->gameplay.MaxActiveAIActors.value();
    for (int i = 0; (i < configLimit) && (i < pickedActorIds.size()); i++) {
        ai_near_actors_ids[i] = pickedActorIds[i];
        pActors[pickedActorIds[i]].attributes |= ACTOR_FULL_AI_STATE;
    }

    ai_arrays_size = std::min(configLimit, (int)pickedActorIds.size());

    return ai_arrays_size;
}

//----- (004070EF) --------------------------------------------------------
bool Detect_Between_Objects(Pid uObjID, Pid uObj2ID) {
    // get object 1 info
    int obj1_pid = uObjID.id();
    int obj1_sector;
    Vec3f pos1;

    switch (uObjID.type()) {
        case OBJECT_Decoration:
            pos1 = pLevelDecorations[obj1_pid].vPosition;
            obj1_sector = pIndoor->GetSector(pos1);
            break;
        case OBJECT_Actor:
            pos1 = pActors[obj1_pid].pos + Vec3f(0, 0, pActors[obj1_pid].height * 0.69999999);
            obj1_sector = pActors[obj1_pid].sectorId;
            break;
        case OBJECT_Sprite:
            pos1 = pSpriteObjects[obj1_pid].vPosition;
            obj1_sector = pSpriteObjects[obj1_pid].uSectorID;
            break;
        default:
            return 0;
    }

    // get object 2 info
    int obj2_pid = uObj2ID.id();
    int obj2_sector;
    Vec3f pos2;

    switch (uObj2ID.type()) {
        case OBJECT_Decoration:
            pos2 = pLevelDecorations[obj2_pid].vPosition;
            obj2_sector = pIndoor->GetSector(pos2);
            break;
        case OBJECT_Character:
            pos2 = pParty->pos + Vec3f(0, 0, pParty->eyeLevel);
            obj2_sector = pBLVRenderParams->uPartyEyeSectorID;
            break;
        case OBJECT_Actor:
            pos2 = pActors[obj2_pid].pos + Vec3f(0, 0, pActors[obj2_pid].height * 0.69999999);
            obj2_sector = pActors[obj2_pid].sectorId;
            break;
        case OBJECT_Sprite:
            pos2 = pSpriteObjects[obj2_pid].vPosition;
            obj2_sector = pSpriteObjects[obj2_pid].uSectorID;
            break;
        default:
            return 0;
    }

    // get distance between objects
    float dist_x = pos2.x - pos1.x;
    float dist_y = pos2.y - pos1.y;
    float dist_z = pos2.z - pos1.z;
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
    BBoxf bbox = BBoxf::forPoints(pos1, pos2);

    // Trace ray through portals from obj1_sector towards obj2_sector.
    int sectors_visited = 0;
    int current_sector = obj1_sector;

    while (sectors_visited < 30) {
        const BLVSector &sector = pIndoor->sectors[current_sector];
        int next_sector = 0;

        for (uint16_t portalId : sector.portalIds) {
            BLVFace *portalface = &pIndoor->faces[portalId];
            Vec3f *portalverts = &pIndoor->vertices[portalface->vertexIds[0]];

            // ray obj1 to portal dot normal
            float obj1portaldot = dot(portalface->facePlane.normal, *portalverts - pos1);

            // flip norm if we are not looking out from current sector
            if (current_sector != portalface->sectorId)
                obj1portaldot = -obj1portaldot;

            // obj1 sees back of, but is not on the portal so skip
            if (obj1portaldot >= 0 && portalverts->x != pos1.x && portalverts->y != pos1.y && portalverts->z != pos1.z)
                continue;

            // bounds check
            if (!bbox.intersects(portalface->boundingBox))
                continue;

            // dot plane normal with obj ray
            float facenotparallel = dot(portalface->facePlane.normal, Vec3f(rayxnorm, rayynorm, rayznorm));

            // if face is parallel == 0 dont check LOS  -- add epsilon?
            if (!facenotparallel)
                continue;

            // point to plane distance
            float pointplanedist = -portalface->facePlane.signedDistanceTo(pos1);

            // epsilon check?
            if (std::abs(pointplanedist) / 16384.0 > std::abs(facenotparallel))
                continue;

            // how far along line intersection is
            float intersect = pointplanedist / facenotparallel;

            // less than zero and intersection is behind target
            if (intersect < 0)
                continue;

            // check if point along ray is in portal face
            Vec3f pos = pos1 + Vec3f(rayxnorm, rayynorm, rayznorm) * intersect;
            if (!portalface->Contains(pos, MODEL_INDOOR))
                continue;

            // get next sector through portal
            if (portalface->sectorId == current_sector)
                next_sector = portalface->backSectorId;
            else
                next_sector = portalface->sectorId;
            break;
        }

        // no portal found leading further
        if (next_sector == 0 || next_sector == current_sector)
            break;

        ++sectors_visited;
        current_sector = next_sector;

        // found object / player / monster
        if (current_sector == obj2_sector)
            return 1;
    }
    // did we stop in the sector where object is?
    if (current_sector != obj2_sector) return 0;
    return 1;
}

//----- (0044FA4C) --------------------------------------------------------
void Spawn_Light_Elemental(int spell_power, Mastery caster_skill_mastery, Duration duration) {
    // size_t uActorIndex;            // [sp+10h] [bp-10h]@6

    const char *cMonsterName;       // [sp-4h] [bp-24h]@2
    if (caster_skill_mastery == MASTERY_GRANDMASTER)
        cMonsterName = "Elemental Light C";
    else if (caster_skill_mastery == MASTERY_MASTER)
        cMonsterName = "Elemental Light B";
    else
        cMonsterName = "Elemental Light A";
    MonsterId uMonsterID = pMonsterList->GetMonsterIDByName(cMonsterName);

    Actor *actor = AllocateActor(false);
    if (!actor)
        return; // Too many actors.

    int partySectorId = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        partySectorId = pBLVRenderParams->uPartySectorID;

    int radius = uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ? 128 : 64;
    int angle = grng->random(2048);

    actor->name = pMonsterStats->infos[uMonsterID].name;
    actor->hp = pMonsterStats->infos[uMonsterID].hp;
    actor->monsterInfo = pMonsterStats->infos[uMonsterID];
    actor->monsterId = uMonsterID;
    actor->radius = pMonsterList->monsters[uMonsterID].monsterRadius;
    actor->height = pMonsterList->monsters[uMonsterID].monsterHeight;
    actor->monsterInfo.goldDiceRolls = 0;
    actor->monsterInfo.treasureType = RANDOM_ITEM_ANY;
    actor->monsterInfo.exp = 0;
    actor->moveSpeed = pMonsterList->monsters[uMonsterID].movementSpeed;
    actor->initialPosition.x = pParty->pos.x + TrigLUT.cos(angle) * radius;
    actor->initialPosition.y = pParty->pos.y + TrigLUT.sin(angle) * radius;
    actor->initialPosition.z = pParty->pos.z;
    actor->pos = actor->initialPosition;
    actor->tetherDistance = 256;
    actor->sectorId = partySectorId;
    actor->PrepareSprites(0);
    actor->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
    actor->ally = MONSTER_TYPE_9999;
    actor->group = 0;
    actor->currentActionTime = 0_ticks;
    actor->aiState = Summoned;
    actor->currentActionLength = 256_ticks;
    actor->UpdateAnimation();

    int sectorId = pIndoor->GetSector(actor->pos);
    int zlevel;
    int zdiff;
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ||
            sectorId == partySectorId &&
            (zlevel = BLV_GetFloorLevel(actor->pos, sectorId), zlevel != -30000) &&
            (zdiff = std::abs(zlevel - pParty->pos.z), zdiff <= 1024)) {
        actor->summonerId = Pid(OBJECT_Character, spell_power);

        actor->buffs[ACTOR_BUFF_SUMMONED].Apply(pParty->GetPlayingTime() + duration,
                                                caster_skill_mastery, spell_power, 0, 0);
    } else {
        actor->Remove();
    }
}

//----- (0044F57C) --------------------------------------------------------
void SpawnEncounter(MapInfo *pMapInfo, SpawnPoint *spawn, int monsterCatMod, int countOverride, int aggro) {
    assert(spawn->uKind == OBJECT_Actor);

    char v8;               // zf@5
    if (GetAlertStatus())
        v8 = (spawn->uAttributes & 1) == 0;
    else
        v8 = (spawn->uAttributes & 1) == 1;
    if (v8) return;

    int NumToSpawn = 1;
    std::string baseTextureName;
    int monsterCategoryOddsSet = 0;
    switch (spawn->uMonsterIndex - 1) {
        case 0:
            monsterCategoryOddsSet = pMapInfo->Dif_M1;
            NumToSpawn = pMapInfo->encounter1MinCount + grng->random(pMapInfo->encounter1MaxCount - pMapInfo->encounter1MinCount + 1);
            baseTextureName = pMapInfo->encounter1MonsterTexture;
            break;
        case 1:
            monsterCategoryOddsSet = pMapInfo->Dif_M2;
            NumToSpawn = pMapInfo->encounter2MinCount + grng->random(pMapInfo->encounter2MaxCount - pMapInfo->encounter2MinCount + 1);
            baseTextureName = pMapInfo->encounter2MonsterTexture;
            break;
        case 2:
            monsterCategoryOddsSet = pMapInfo->Dif_M3;
            NumToSpawn = pMapInfo->encounter3MinCount + grng->random(pMapInfo->encounter3MaxCount - pMapInfo->encounter3MinCount + 1);
            baseTextureName = pMapInfo->encounter3MonsterTexture;
            break;
        case 3:
            baseTextureName = pMapInfo->encounter1MonsterTexture + " A";
            break;
        case 4:
            baseTextureName = pMapInfo->encounter2MonsterTexture + " A";
            break;
        case 5:
            baseTextureName = pMapInfo->encounter3MonsterTexture + " A";
            break;
        case 6:
            baseTextureName = pMapInfo->encounter1MonsterTexture + " B";
            break;
        case 7:
            baseTextureName = pMapInfo->encounter2MonsterTexture + " B";
            break;
        case 8:
            baseTextureName = pMapInfo->encounter3MonsterTexture + " B";
            break;
        case 9:
            baseTextureName = pMapInfo->encounter1MonsterTexture + " C";
            break;
        case 10:
            baseTextureName = pMapInfo->encounter2MonsterTexture + " C";
            break;
        case 11:
            baseTextureName = pMapInfo->encounter3MonsterTexture + " C";
            break;
        default:
            return;
    }

    if (baseTextureName[0] == '0') return;

    monsterCategoryOddsSet += monsterCatMod;
    if (monsterCategoryOddsSet > 3) monsterCategoryOddsSet = 3;

    if (countOverride) NumToSpawn = countOverride;
    if (NumToSpawn <= 0) return;

    // Config multiplier now
    NumToSpawn = std::ceil(NumToSpawn * engine->config->gameplay.SpawnCountMultiplier.value());
    NumToSpawn = std::clamp(NumToSpawn, 1, engine->config->gameplay.MaxActors.value());

    int pSector = 0;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        pSector = pIndoor->GetSector(spawn->vPosition);

    // spawning loop
    std::string finalTextureName = baseTextureName;
    for (int i = 0; i < NumToSpawn; ++i) {
        Actor *pMonster = AllocateActor(true);
        if (!pMonster)
            continue;

        // random monster levels ABC
        if (monsterCategoryOddsSet) {
            int catRandom = grng->random(100);
            int finalCat = 3;  // 2 , 10 , 20 - C
            int lowThresh = word_4E8152[3 * monsterCategoryOddsSet];
            if (catRandom >= lowThresh) {
                if (catRandom < lowThresh + word_4E8152[3 * monsterCategoryOddsSet + 1]) {
                    finalCat = 2;  // 8 , 20 , 30 - B
                }
            } else {
                finalCat = 1;  // 90 , 70 , 50 - A
            }

            if (finalCat == 1) {
                finalTextureName = baseTextureName + " A";
            } else if (finalCat == 2) {
                finalTextureName = baseTextureName + " B";
            } else {
                finalTextureName = baseTextureName + " C";
            }
        }

        MonsterId monsterDescID = pMonsterList->GetMonsterIDByName(finalTextureName);
        MonsterDesc* monsterDesc = &pMonsterList->monsters[monsterDescID];
        MonsterId monster = pMonsterStats->FindMonsterByTextureName(finalTextureName);

        // TODO(captainurist): MONSTER_ANGEL_A is monster #1, why do we even need this check?
        if (monster == MONSTER_INVALID) monster = MONSTER_ANGEL_A;

        MonsterInfo* Src = &pMonsterStats->infos[monster];
        pMonster->name = Src->name;
        pMonster->hp = Src->hp;
        pMonster->monsterInfo = pMonsterStats->infos[monster];
        pMonster->monsterId = monsterDescID;
        pMonster->radius = monsterDesc->monsterRadius;
        pMonster->height = monsterDesc->monsterHeight;
        pMonster->moveSpeed = monsterDesc->movementSpeed;
        pMonster->initialPosition = spawn->vPosition;
        pMonster->pos = spawn->vPosition;
        pMonster->tetherDistance = 256;
        pMonster->sectorId = pSector;
        pMonster->group = spawn->uGroup;
        pMonster->PrepareSprites(0);
        pMonster->monsterInfo.hostilityType = HOSTILITY_FRIENDLY;

        // TODO(pskelton): We calculate a new position for the monster, but we never use it.
        int randomAngle = grng->random(2048);
        int distance = (((uCurrentlyLoadedLevelType != LEVEL_OUTDOOR) - 1) & 0x40) + 64; // 64 indoor or 128 outdoor

        Vec3f newPos;
        newPos.x = TrigLUT.cos(randomAngle) * distance + spawn->vPosition.x;
        newPos.y = TrigLUT.sin(randomAngle) * distance + spawn->vPosition.y;
        newPos.z = spawn->vPosition.z;

        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            if (aggro)
                pMonster->attributes |= ACTOR_AGGRESSOR;
            continue;
        }

        int newSector = pIndoor->GetSector(newPos);
        if (newSector == pSector) {
            int newFloorLevel = BLV_GetFloorLevel(newPos, newSector);
            if (newFloorLevel != -30000) {
                if (std::abs(newFloorLevel - newPos.z) <= 1024) {
                    newPos.z = newFloorLevel;
                    if (aggro)
                        pMonster->attributes |= ACTOR_AGGRESSOR;
                    continue;
                }
            }
        }

        // TODO(pskelton): with above position should we either retry or fallback to spawn pos
        // Cant just remove actors as they appear killed see #2074
        // Actor was spawned too far away, remove it.
        //pMonster->Remove();
    }
}

void evaluateAoeDamage() {
    SpriteObject *pSpriteObj = nullptr;

    for (AttackDescription &attack : attackList) {
        ObjectType attackerType = attack.pid.type();
        int attackerId = attack.pid.id();

        // attacker is an item (sprite)
        if (attackerType == OBJECT_Sprite) {
            pSpriteObj = &pSpriteObjects[attackerId];
            attackerType = pSpriteObjects[attackerId].spell_caster_pid.type();
            attackerId = pSpriteObjects[attackerId].spell_caster_pid.id();
            // This is triggered by the rock blast decorations Armageddon spawns.
            // If let through, they can trigger the assert near the end of the loop.
            if (pSpriteObj->uType == SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT && attackerType == OBJECT_None)
                continue;
        }

        if (attack.isMelee) {
            unsigned int targetId = ai_near_actors_targets_pid[attackerId].id();
            ObjectType targetType = ai_near_actors_targets_pid[attackerId].type();
            Actor *actor = &pActors[targetId];

            if (targetType != OBJECT_Actor) {
                if (targetType == OBJECT_Character) {  // party damage from monsters
                    int distanceSq = (pParty->pos + Vec3f(0, 0, pParty->height / 2) - attack.pos).lengthSqr();
                    int attackRangeSq = (attack.attackRange + 32) * (attack.attackRange + 32);

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(pParty->pos + Vec3f(0, 0, pParty->eyeLevel), attack.pos)) {
                            DamageCharacterFromMonster(attack.pid, attack.attackSpecial, stru_50C198.which_player_to_attack(&pActors[attackerId]));
                        }
                    }
                }
            } else {  // Actor (peasant) damage from monsters
                if (actor->buffs[ACTOR_BUFF_PARALYZED].Active() || actor->CanAct()) {
                    Vec3f distanceVec = actor->pos + Vec3f(0, 0, actor->height / 2) - attack.pos;
                    float distanceSq = distanceVec.lengthSqr();
                    float attackRange = attack.attackRange + actor->radius;
                    float attackRangeSq = attackRange * attackRange;
                    Vec3f attackVector = Vec3f(distanceVec.x, distanceVec.y, actor->pos.z);
                    attackVector.normalize();

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(actor->pos + Vec3f(0, 0, 50), attack.pos)) {
                            Actor::ActorDamageFromMonster(attack.pid, targetId, attackVector, attack.attackSpecial);
                        }
                    }
                }
            }
        } else {  // damage from AOE spells
            int distanceSq = (pParty->pos + Vec3f(0, 0, pParty->height / 2) - attack.pos).lengthSqr();
            int attackRangeSq = (attack.attackRange + 32) * (attack.attackRange + 32);

            // check spell in range of party
            if (distanceSq < attackRangeSq) {  // party damage
                // check line of sight to party
                if (Check_LineOfSight(pParty->pos + Vec3f(0, 0, pParty->eyeLevel), attack.pos)) {
                    for (int i = 0; i < pParty->pCharacters.size(); i++) {
                        if (pParty->pCharacters[i].conditions.hasNone({CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
                            DamageCharacterFromMonster(attack.pid, attack.attackSpecial, i);
                        }
                    }
                }
            }

            for (int actorID = 0; actorID < pActors.size(); ++actorID) {
                if (pActors[actorID].CanAct()) {
                    Vec3f distanceVec = pActors[actorID].pos + Vec3f(0, 0, pActors[actorID].height / 2) - attack.pos;
                    float distanceSq = distanceVec.lengthSqr();
                    float attackRange = attack.attackRange + pActors[actorID].radius;
                    float attackRangeSq = attackRange * attackRange;
                    // TODO: using absolute Z here is BS, it's used as speed in ItemDamageFromActor
                    Vec3f attVF = Vec3f(distanceVec.x, distanceVec.y, pActors[actorID].pos.z);
                    attVF.normalize();

                    // check range
                    if (distanceSq < attackRangeSq) {
                        // check line of sight
                        if (Check_LineOfSight(pActors[actorID].pos + Vec3f(0, 0, 50), attack.pos)) {
                            switch (attackerType) {
                                case OBJECT_Character:
                                    Actor::DamageMonsterFromParty(attack.pid, actorID, attVF);
                                    break;
                                case OBJECT_Actor:
                                    if (pSpriteObj && pActors[attackerId].GetActorsRelation(&pActors[actorID]) != HOSTILITY_FRIENDLY) {
                                        Actor::ActorDamageFromMonster(attack.pid, actorID, attVF, pSpriteObj->spellCasterAbility);
                                    }
                                    break;
                                case OBJECT_Sprite:
                                    ItemDamageFromActor(attack.pid, actorID, attVF);
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
void ItemDamageFromActor(Pid uObjID, unsigned int uActorID, const Vec3f &pVelocity) {
     if (!pActors[uActorID].IsNotAlive()) {
        if (uObjID.type() == OBJECT_Sprite) {
            if (pSpriteObjects[uObjID.id()].uSpellID != SPELL_NONE) {
                int spellDamage = CalcSpellDamage(
                    pSpriteObjects[uObjID.id()].uSpellID,
                    pSpriteObjects[uObjID.id()].spell_level,
                    pSpriteObjects[uObjID.id()].spell_skill,
                    pActors[uActorID].hp);
                int damage = pActors[uActorID].CalcMagicalDamageToActor(DAMAGE_FIRE, spellDamage);
                pActors[uActorID].hp -= damage;

                if (damage > 0) {
                    if (pActors[uActorID].hp > 0)
                        Actor::AI_Stun(uActorID, uObjID, 0);
                    else
                        Actor::Die(uActorID);

                    int knockback = 20 * damage / (signed int)pActors[uActorID].monsterInfo.hp;
                    if (knockback > 10)
                        knockback = 10;
                    if (supertypeForMonsterId(pActors[uActorID].monsterInfo.id) != MONSTER_SUPERTYPE_TREANT) {
                        pActors[uActorID].velocity = 50 * knockback * pVelocity;
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

    if (pActors.size() >= engine->config->gameplay.MaxActors.value())
        return nullptr; // Too many actors.

    return &pActors.emplace_back(Actor(pActors.size()));
}
