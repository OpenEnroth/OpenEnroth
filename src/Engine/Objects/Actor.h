#pragma once

#include <vector>
#include <string>

#include "Engine/Spells/SpellBuff.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/CombinedSkillValue.h"

#include "Utility/Geometry/Vec.h"
#include "Utility/IndexedArray.h"

#include "ActorEnums.h"

class Actor;
class Vis;
struct SpawnPoint;

struct stru319 {
    stru319();

    int which_player_to_attack(Actor *pActor);
    int _427546(int a2);
    int FindClosestActor(int pick_depth, int a3, int target_undead);

    char field_0 = 0;

    Vis *vis = nullptr;
};

extern stru319 stru_50C198;  // idb

struct AIDirection {
    Vec3i vDirection{};
    unsigned int uDistance = 0;
    unsigned int uDistanceXZ = 0;
    unsigned int uYawAngle = 0;
    /*un*/signed int uPitchAngle = 0;
};

struct ActorJob {
    Vec3s vPos;
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
    signed int GetActorsRelation(Actor *a2);
    void SetRandomGoldIfTheresNoItem();
    bool CanAct();
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

    static void _SelectTarget(unsigned int uActorID, int *OutTargetPID,
                              bool can_target_party);
    static void AI_Pursue3(unsigned int uActorID, unsigned int a2,
                           signed int uActionLength, struct AIDirection *a4);
    static void AI_Pursue2(unsigned int uActorID, unsigned int a2,
                           signed int uActionLength, struct AIDirection *pDir,
                           int a5);
    static void AI_Flee(unsigned int uActorID, signed int edx0,
                        int uActionLength, struct AIDirection *a4);
    static void AI_Pursue1(unsigned int uActorID, unsigned int a2,
                           signed int arg0, signed int uActionLength,
                           struct AIDirection *pDir);
    /**
     * @offset 0x402CED
     */
    static void playSound(unsigned int uActorID, ActorSounds uSoundID);
    static void Die(unsigned int uActorID);
    static void resurrect(unsigned int uActorID);
    static void AI_Bored(unsigned int uActorID, unsigned int uObjID,
                         struct AIDirection *a4);
    static void AI_Stun(unsigned int uActorID, signed int edx0, int arg0);
    static char _4031C1_update_job_never_gets_called(unsigned int uActorID,
                                                     signed int a2, int a3);
    static void AI_RandomMove(unsigned int uActor_id, unsigned int uTarget_id,
                              int radius, int uActionLength);
    static void AI_MissileAttack1(unsigned int uActorID, signed int sTargetPid,
                                  struct AIDirection *pDir);
    static void AI_MissileAttack2(unsigned int uActorID, signed int sTargetPid,
                                  struct AIDirection *pDir);
    static void AI_SpellAttack1(unsigned int uActorID, signed int sTargetPid,
                                struct AIDirection *pDir);
    static void AI_SpellAttack2(unsigned int uActorID, signed int sTargetPid,
                                struct AIDirection *pDir);
    static void AI_MeleeAttack(unsigned int uActorID, signed int sTargetPid,
                               struct AIDirection *arg0);
    static void StandAwhile(unsigned int uActorID);
    static void AI_Stand(unsigned int uActorID, unsigned int object_to_face_pid,
                         unsigned int uActionLength, struct AIDirection *a4);
    static void AI_StandOrBored(unsigned int uActorID, signed int uObjID,
                                int uActionLength, struct AIDirection *a4);
    static void AI_FaceObject(unsigned int uActorID, unsigned int uObjID,
                              int UNUSED, struct AIDirection *Dir_In);
    static void GetDirectionInfo(unsigned int uObj1ID, unsigned int uObj2ID,
                                 struct AIDirection *pOut, int a4);
    static void Explode(unsigned int uActorID);
    static void AI_RangedAttack(unsigned int uActorID, struct AIDirection *a2,
                                int type, ABILITY_INDEX a4);
    static void AI_SpellAttack(unsigned int uActorID, struct AIDirection *pDir,
                               SPELL_TYPE uSpellID, ABILITY_INDEX a4, CombinedSkillValue uSkill);
    static void ActorDamageFromMonster(int attacker_id, unsigned int actor_id,
                                       Vec3i *pVelocity, ABILITY_INDEX a4);

    static unsigned short GetObjDescId(SPELL_TYPE spellId);

    static void AggroSurroundingPeasants(unsigned int uActorID, int a2);
    static bool ArePeasantsOfSameFaction(Actor *a1, Actor *a2);
    static void StealFrom(unsigned int uActorID);

    /**
     * @offset 0x4485A7
     */
    static void giveItem(signed int uActorID, ITEM_TYPE uItemID, unsigned int bGive);

    /**
     * @offset 0x448A40
     */
    static void toggleFlag(signed int uActorID, ActorAttribute uFlag, bool bToggle);
    static void ApplyFineForKillingPeasant(unsigned int uActorID);
    static void DrawHealthBar(Actor *actor, GUIWindow *window);
    int _43B3E0_CalcDamage(ABILITY_INDEX dmgSource);
    static void AddOnDamageOverlay(unsigned int uActorID, int overlayType, int damage);

    static void Arena_summon_actor(int monster_id, int x, int y, int z);
    static void DamageMonsterFromParty(int a1, unsigned int uActorID_Monster,
                                       Vec3i *pVelocity);
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
    static bool isActorKilled(ACTOR_KILL_CHECK_POLICY policy, int param, int count);

    /**
     * @offset 0x408A27
     */
    static int searchDeadActors(int *pTotalActors);

    /**
     * @offset 0x408A7E
     */
    static int searchDeadActorsByMonsterID(int *pTotalActors, int monsterID);

    /**
     * @offset 0x408AE7
     */
    static int searchDeadActorsByGroup(int *pTotalActors, int group);

    /**
     * @offset 0x408B54
     */
    static int searchDeadActorsByID(int *pTotalActors, int id);

    void LootActor();
    bool _427102_IsOkToCastSpell(SPELL_TYPE spell);
    ABILITY_INDEX special_ability_use_check(int a2);
    bool _4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4);
    bool ActorHitOrMiss(Character *pPlayer);
    int CalcMagicalDamageToActor(DAMAGE_TYPE dmgType, int incomingDmg);
    bool DoesDmgTypeDoDamage(DAMAGE_TYPE uType);

    int id = -1; // Actor index in pActors array.
    std::string name;
    int16_t npcId = 0;
    ActorAttributes attributes = 0;
    int16_t currentHP = 0;
    MonsterInfo monsterInfo;
    int16_t word_000084_range_attack = 0;
    int16_t word_000086_some_monster_id = 0;  // base monster class monsterlist id
    uint16_t radius = 32;
    uint16_t height = 128;
    uint16_t moveSpeed = 200;
    Vec3s pos;
    Vec3s speed;
    uint16_t yawAngle = 0;
    uint16_t pitchAngle = 0;
    int sectorId = 0;
    uint16_t currentActionLength = 0;
    Vec3s initialPosition;
    Vec3s guardingPosition;
    uint16_t tetherDistance = 256;
    AIState aiState = Standing;
    ActorAnimation currentActionAnimation = ANIM_Standing;
    ITEM_TYPE carriedItemId = ITEM_NULL; // carried items are special items the
                                         // ncp carries (ie lute from bard)
    unsigned int currentActionTime = 0;
    IndexedArray<uint16_t, ANIM_First, ANIM_Last> spriteIds = {{}};
    std::array<uint16_t, 4> soundSampleIds = {{}};  // 1 die     3 bored
    IndexedArray<SpellBuff, ACTOR_BUFF_FIRST, ACTOR_BUFF_LAST> buffs;
    std::array<ItemGen, 4> items;
    unsigned int group = 0;
    unsigned int ally = 0;
    std::array<ActorJob, 8> scheduledJobs;
    unsigned int summonerId = 0;
    unsigned int lastCharacterIdToHit = 0;
    int dword_000334_unique_name = 0; // Index into pMonsterStats->pPlaceStrings for a unique monster name.
    bool donebloodsplat{ false };
};

extern std::vector<Actor> pActors;

bool CheckActors_proximity();

/**
 * @offset 0x448518
 */
void npcSetItem(int npc, ITEM_TYPE item, int a3);

/**
 * @offset 0x448A98
 */
void toggleActorGroupFlag(unsigned int uGroupID, ActorAttribute uFlag, bool bValue);
bool Detect_Between_Objects(unsigned int uObjID, unsigned int uObj2ID);
bool SpawnActor(unsigned int uMonsterID);
void Spawn_Light_Elemental(int spell_power, CharacterSkillMastery caster_skill_mastery, int duration_game_seconds);
void SpawnEncounter(struct MapInfo *pMapInfo, SpawnPoint *spawn, int a3, int a4, int a5);
/**
 * @offset 0x438F8F
 */
void evaluateAoeDamage();
double sub_43AE12(signed int a1);
void ItemDamageFromActor(unsigned int uObjID, unsigned int uActorID,
                         Vec3i *pVelocity);

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
