#pragma once

#include <deque>
#include <string>

#include "Engine/Spells/SpellBuff.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Pid.h"

#include "Media/Audio/SoundEnums.h"

#include "Library/Geometry/Vec.h"

#include "Utility/IndexedArray.h"

#include "ActorEnums.h"

class Actor;
class Vis;
struct SpawnPoint;
struct MapInfo;

struct stru319 {
    int which_player_to_attack(Actor *pActor);
    int _427546(int a2);
    Pid FindClosestActor(int pick_depth, int a3, int target_undead);
};

extern stru319 stru_50C198;  // idb

struct AIDirection {
    Vec3f vDirection;
    float uDistance = 0;
    float uDistanceXZ = 0;
    unsigned int uYawAngle = 0;
    /*un*/signed int uPitchAngle = 0;
};

struct ActorJob {
    Vec3f vPos;
    uint16_t uAttributes = 0;
    uint8_t uAction = 0;
    uint8_t uHour = 0;
    uint8_t uDay = 0;
    uint8_t uMonth = 0;
};

class GUIWindow;

class Actor {
 public:
    Actor() {}
    explicit Actor(int id): id(id) {}

    void SummonMinion(int summonerId);
    void Reset();
    void Remove();
    void PrepareSprites(char load_sounds_if_bit1_set);
    void UpdateAnimation();
    MonsterHostility GetActorsRelation(Actor *a2);
    void SetRandomGoldIfTheresNoItem();
    bool CanAct() const;
    bool IsNotAlive();
    bool IsPeasant();

    inline void ResetAnimation() { attributes &= ~ACTOR_ANIMATION; }
    inline void ResetQueue() { attributes &= ~ACTOR_STAND_IN_QUEUE; }
    inline void ResetActive() { attributes &= ~ACTOR_ACTIVE; }
    inline void ResetFullAiState() { attributes &= ~ACTOR_FULL_AI_STATE; }
    inline void ResetHasItem() { attributes &= ~ACTOR_HAS_ITEM; }
    inline void ResetHostile() { attributes &= ~ACTOR_HOSTILE; }
    inline void ResetAggressor() { attributes &= ~ACTOR_AGGRESSOR; }
    inline bool ActorEnemy() const {
        return attributes & ACTOR_AGGRESSOR;
    }
    inline bool ActorFriend() const {
        return !(attributes & ACTOR_AGGRESSOR);
    }
    inline bool ActorHasItem() const {
        return attributes & ACTOR_HAS_ITEM;
    }
    inline bool ActorNearby() const {
        return attributes & ACTOR_NEARBY;
    }

    static void _SelectTarget(unsigned int uActorID, Pid *OutTargetPID,
                              bool can_target_party);
    static void AI_Pursue3(unsigned int uActorID, Pid a2,
                           Duration uActionLength, AIDirection *a4);
    static void AI_Pursue2(unsigned int uActorID, Pid a2,
                           Duration uActionLength, AIDirection *pDir,
                           int a5);
    static void AI_Flee(unsigned int uActorID, Pid edx0,
                        Duration uActionLength, AIDirection *a4);
    static void AI_Pursue1(unsigned int uActorID, Pid a2,
                           signed int arg0, Duration uActionLength,
                           AIDirection *pDir);
    /**
     * @offset 0x402CED
     */
    static void playSound(unsigned int uActorID, ActorSound uSoundID);
    static void Die(unsigned int uActorID);
    static void resurrect(unsigned int uActorID);
    static void AI_Bored(unsigned int uActorID, Pid uObjID,
                         AIDirection *a4);
    static void AI_Stun(unsigned int uActorID, Pid edx0, int arg0);
    static char _4031C1_update_job_never_gets_called(unsigned int uActorID,
                                                     signed int a2, int a3);
    static void AI_RandomMove(unsigned int uActor_id, Pid uTarget_id,
                              int radius, Duration uActionLength);
    static void AI_MissileAttack1(unsigned int uActorID, Pid sTargetPid,
                                  AIDirection *pDir);
    static void AI_MissileAttack2(unsigned int uActorID, Pid sTargetPid,
                                  AIDirection *pDir);
    static void AI_SpellAttack1(unsigned int uActorID, Pid sTargetPid,
                                AIDirection *pDir);
    static void AI_SpellAttack2(unsigned int uActorID, Pid sTargetPid,
                                AIDirection *pDir);
    static void AI_MeleeAttack(unsigned int uActorID, Pid sTargetPid,
                               AIDirection *arg0);
    static void StandAwhile(unsigned int uActorID);
    static void AI_Stand(unsigned int uActorID, Pid object_to_face_pid,
                         Duration uActionLength, AIDirection *a4);
    static void AI_StandOrBored(unsigned int uActorID, Pid uObjID,
                                Duration uActionLength, AIDirection *a4);
    static void AI_FaceObject(unsigned int uActorID, Pid uObjID, AIDirection *Dir_In);

    /**
    * @param p1                            From Vec3f.
    * @param p2                            To Vec3f.
    * @param[out] pOut                     AIDirection from p1 to p2.
    */
    static void GetDirectionInfo(Vec3f p1, Vec3f p2, AIDirection* pOut);
    static void GetDirectionInfo(Pid uObj1ID, Pid uObj2ID,
                                 AIDirection *pOut, int PreferedZ);
    static void Explode(unsigned int uActorID);
    static void AI_RangedAttack(unsigned int uActorID, AIDirection *a2,
                                int type, ActorAbility a4);
    static void AI_SpellAttack(unsigned int uActorID, AIDirection *pDir,
                               SpellId uSpellID, ActorAbility a4, CombinedSkillValue uSkill);
    static void ActorDamageFromMonster(Pid attacker_id, unsigned int actor_id,
                                       const Vec3f &pVelocity, ActorAbility a4);

    static unsigned short GetObjDescId(SpellId spellId);

    static void AggroSurroundingPeasants(unsigned int uActorID, int a2);
    static bool ArePeasantsOfSameFaction(Actor *a1, Actor *a2);
    static void StealFrom(unsigned int uActorID);

    /**
     * @offset 0x4485A7
     */
    static void giveItem(signed int uActorID, ItemId uItemID, unsigned int bGive);

    /**
     * @offset 0x448A40
     */
    static void toggleFlag(signed int uActorID, ActorAttribute uFlag, bool bToggle);
    static void ApplyFineForKillingPeasant(unsigned int uActorID);
    static void DrawHealthBar(Actor *actor, GUIWindow *window);
    int _43B3E0_CalcDamage(ActorAbility dmgSource);
    static void AddOnDamageOverlay(unsigned int uActorID, int overlayType, int damage);

    static void Arena_summon_actor(MonsterId monster_id, Vec3f pos);
    static int DamageMonsterFromParty(Pid a1, unsigned int uActorID_Monster, const Vec3f &pVelocity);
    static void MakeActorAIList_ODM();
    static int MakeActorAIList_BLV();
    static void UpdateActorAI();
    static void InitializeActors();

    /**
     * @param policy    Determines type for actors check.
     * @param param     Parameter determined by check policy.
     * @param count     Killed count or 0 if all must be killed
     * @offset 0x44665D
     */
    static bool isActorKilled(ActorKillCheckPolicy policy, int param, int count);

    /**
     * @offset 0x408A27
     */
    static int searchDeadActors(int *pTotalActors);

    /**
     * @offset 0x408A7E
     */
    static int searchDeadActorsByMonsterID(int *pTotalActors, MonsterId monsterID);

    /**
     * @offset 0x408AE7
     */
    static int searchDeadActorsByGroup(int *pTotalActors, int group);

    /**
     * @offset 0x408B54
     */
    static int searchDeadActorsByID(int *pTotalActors, int id);

    void LootActor();
    bool _427102_IsOkToCastSpell(SpellId spell);
    ActorAbility special_ability_use_check(int a2);
    bool _4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4);
    bool ActorHitOrMiss(Character *pPlayer);
    int CalcMagicalDamageToActor(DamageType dmgType, int incomingDmg);
    bool DoesDmgTypeDoDamage(DamageType uType);

    int id = -1; // Actor index in pActors array.
    std::string name;
    int16_t npcId = 0;
    ActorAttributes attributes = 0;
    int16_t currentHP = 0;
    MonsterInfo monsterInfo;
    int16_t word_000084_range_attack = 0;
    MonsterId word_000086_some_monster_id = MONSTER_INVALID;  // base monster class monsterlist id
    uint16_t radius = 32;
    uint16_t height = 128;
    uint16_t moveSpeed = 200;
    Vec3f pos;
    Vec3f velocity;
    uint16_t yawAngle = 0;
    uint16_t pitchAngle = 0;
    int sectorId = 0;
    Duration currentActionLength = 0_ticks;
    Vec3f initialPosition;
    Vec3f guardingPosition;
    uint16_t tetherDistance = 256;
    AIState aiState = Standing;
    ActorAnimation currentActionAnimation = ANIM_Standing;
    ItemId carriedItemId = ITEM_NULL; // carried items are special items the
                                         // ncp carries (ie lute from bard)
    Duration currentActionTime = 0_ticks;
    IndexedArray<uint16_t, ANIM_First, ANIM_Last> spriteIds = {{}};
    IndexedArray<SoundId, ACTOR_SOUND_FIRST, ACTOR_SOUND_LAST> soundSampleIds = {{}};
    IndexedArray<SpellBuff, ACTOR_BUFF_FIRST, ACTOR_BUFF_LAST> buffs;
    std::array<Item, 4> items;
    unsigned int group = 0;
    MonsterType ally = MONSTER_TYPE_INVALID; // TODO(captainurist): document properly, and maybe rename.
    std::array<ActorJob, 8> scheduledJobs;
    Pid summonerId;
    Pid lastCharacterIdToHit;
    int uniqueNameIndex = 0; // Index into pMonsterStats->pUniqueNames for a unique monster name. Regular name is used if this field is 0.
    bool donebloodsplat{ false };
    Duration massDistortionTime; // Value of pMiscTimer when mass distortion was cast. This was stored in the buffs table
                                 // in vanilla, which made little sense. Buff table stores game time, putting a value of
                                 // a misc timer in there is very questionable.
};

extern std::deque<Actor> pActors;

bool CheckActors_proximity();

/**
 * @offset 0x448518
 */
void npcSetItem(int npc, ItemId item, int a3);

/**
 * @offset 0x448A98
 */
void toggleActorGroupFlag(unsigned int uGroupID, ActorAttribute uFlag, bool bValue);
bool Detect_Between_Objects(Pid uObjID, Pid uObj2ID);
void Spawn_Light_Elemental(int spell_power, CharacterSkillMastery caster_skill_mastery, Duration duration);
void SpawnEncounter(MapInfo *pMapInfo, SpawnPoint *spawn, int a3, int a4, int a5);
/**
 * @offset 0x438F8F
 */
void evaluateAoeDamage();
double sub_43AE12(signed int a1);
void ItemDamageFromActor(Pid uObjID, unsigned int uActorID, const Vec3f &pVelocity);

// TODO: in original binary almost all calls are with appendOnly=true, only Spawn_Light_Elemental uses
// appendOnly=false. And this actually makes sense as actor ids can be stored in all kinds of places (e.g. inside
// projectiles as a reference to the caster), so in practice reusing actor ids will lead to rare bugs.
// What can we do with this?
/**
 * Allocates a new actor in `pActors` array.
 *
 * @param appendOnly                    If true, this function doesn't try to find an empty (removed) spot in the
 *                                      actors list, and only appends new actors to the end.
 * @return                              Pointer to a newly allocated actor, or `nullptr` if the actor count limit
 *                                      has been hit.
 */
Actor *AllocateActor(bool appendOnly);
