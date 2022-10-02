#pragma once

#include <vector>
#include <string>

#include "Engine/IocContainer.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/Monsters.h"

#include "Utility/IndexedArray.h"

using EngineIoc = Engine_::IocContainer;
class Actor;

/*  357 */
#pragma pack(push, 1)
struct stru319 {
    inline stru319() {
        this->vis = EngineIoc::ResolveVis();
    }

    int which_player_to_attack(Actor *pActor);
    int _427546(int a2);
    int FindClosestActor(int pick_depth, int a3, int target_undead);

    char field_0 = 0;

    Vis *vis = nullptr;
};
#pragma pack(pop)

extern stru319 stru_50C198;  // idb

enum class ABILITY_INDEX : char {
    ABILITY_ATTACK1 = 0,
    ABILITY_ATTACK2 = 1,
    ABILITY_SPELL1 = 2,
    ABILITY_SPELL2 = 3,
    ABILITY_SPECIAL = 4, // E.g. gogs exploding on death
    ABILITY_UNKNOWN = 5, // TODO: what is this? Can we just drop it?
};
using enum ABILITY_INDEX;

/*  361 */
enum class ACTOR_BUFF_INDEX {
    ACTOR_BUFF_CHARM = 1,
    ACTOR_BUFF_SUMMONED = 2,
    ACTOR_BUFF_SHRINK = 3,
    ACTOR_BUFF_AFRAID = 4,
    ACTOR_BUFF_STONED = 5,
    ACTOR_BUFF_PARALYZED = 6,
    ACTOR_BUFF_SLOWED = 7,
    ACTOR_BUFF_SOMETHING_THAT_HALVES_AC = 8,  // suspect gm axe effect??
    ACTOR_BUFF_BERSERK = 9,
    ACTOR_BUFF_MASS_DISTORTION = 10,
    ACTOR_BUFF_FATE = 11,
    ACTOR_BUFF_ENSLAVED = 12,
    ACTOR_BUFF_DAY_OF_PROTECTION = 13,
    ACTOR_BUFF_HOUR_OF_POWER = 14,
    ACTOR_BUFF_SHIELD = 15,
    ACTOR_BUFF_STONESKIN = 16,
    ACTOR_BUFF_BLESS = 17,
    ACTOR_BUFF_HEROISM = 18,
    ACTOR_BUFF_HASTE = 19,
    ACTOR_BUFF_PAIN_REFLECTION = 20,
    ACTOR_BUFF_PAIN_HAMMERHANDS = 21,
    ACTOR_BUFF_COUNT
};
using enum ACTOR_BUFF_INDEX;

/*  295 */
enum class ObjectType {
    OBJECT_Any = 0x0,
    OBJECT_BLVDoor = 0x1,
    OBJECT_Item = 0x2,
    OBJECT_Actor = 0x3,
    OBJECT_Player = 0x4,
    OBJECT_Decoration = 0x5,
    OBJECT_BModel = 0x6,
};
using enum ObjectType;

/*  264 */
enum AIState : unsigned __int16 {
    Standing = 0x0,
    Tethered = 0x1,
    AttackingMelee = 0x2,
    AttackingRanged1 = 0x3,
    Dying = 0x4,
    Dead = 0x5,
    Pursuing = 0x6,
    Fleeing = 0x7,
    Stunned = 0x8,
    Fidgeting = 0x9,
    Interacting = 10,
    Removed = 11,
    AttackingRanged2 = 0xC,
    AttackingRanged3 = 0xD,
    Stoned = 0xE,
    Paralyzed = 0xF,
    Resurrected = 16,
    Summoned = 17,
    AttackingRanged4 = 18,
    Disabled = 19,
};

/*  265 */
enum ActorAnimation : __int32 {
    ANIM_Standing = 0x0,
    ANIM_Walking = 0x1,
    ANIM_AtkMelee = 0x2,
    ANIM_AtkRanged = 0x3,
    ANIM_GotHit = 0x4,
    ANIM_Dying = 0x5,
    ANIM_Dead = 0x6,
    ANIM_Bored = 0x7,
};

#define ACTOR_VISIBLE 0x00000008
#define ACTOR_STAND_IN_QUEUE 0x00000080

// nearest 30 actors to the players are put into this state - where the ai will attack/ flee/ cast spells/ wander
// out of this state actors will idle on the spot
#define ACTOR_FULL_AI_STATE 0x00000400

#define ACTOR_ACTIVE 0x00004000  // actor ai has been activated
#define ACTOR_NEARBY 0x00008000  // detected player / monster
#define ACTOR_UNKNOW11 0x00010000  // aidisabled?
#define ACTOR_FLEEING 0x00020000
#define ACTOR_UNKNOW5 0x00040000  // casting??
#define ACTOR_AGGRESSOR 0x00080000
#define ACTOR_UNKNOW7 0x00100000  // alertstatus ?
#define ACTOR_ANIMATION 0x00200000
#define ACTOR_UNKNOW9 0x00400000  // actor has job - never finished feature
#define ACTOR_HAS_ITEM 0x00800000
#define ACTOR_HOSTILE 0x01000000

/*  247 */
#pragma pack(push, 1)
struct AIDirection {
    Vec3i vDirection{};
    unsigned int uDistance = 0;
    unsigned int uDistanceXZ = 0;
    unsigned int uYawAngle = 0;
    /*un*/signed int uPitchAngle = 0;
};
#pragma pack(pop)

/*   71 */
#pragma pack(push, 1)
struct ActorJob {
    Vec3s vPos;
    unsigned __int16 uAttributes = 0;
    unsigned __int8 uAction = 0;
    unsigned __int8 uHour = 0;
    unsigned __int8 uDay = 0;
    unsigned __int8 uMonth = 0;
};
#pragma pack(pop)

class GUIWindow;

/*   66 */
#pragma pack(push, 1)
class Actor {
 public:
    Actor() {}
    Actor(int id): id(id) {}

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

    inline void ResetAnimation() { uAttributes &= 0xFFDFFFFF; }
    inline void ResetQueue() { uAttributes &= ~ACTOR_STAND_IN_QUEUE; }
    inline void ResetActive() { uAttributes &= 0xFFFFBFFF; }
    inline void ResetFullAiState() { uAttributes &= ~ACTOR_FULL_AI_STATE; }
    inline void ResetHasItem() { uAttributes &= 0xFF7FFFFF; }
    inline void ResetHostile() { uAttributes &= 0xFEFFFFFF; }
    inline void ResetAggressor() { uAttributes &= 0xFFF7FFFF; }
    inline bool ActorEnemy() const {
        return (uAttributes & ACTOR_AGGRESSOR) != 0;
    }
    inline bool ActorFriend() const {
        return (uAttributes & ACTOR_AGGRESSOR) == 0;
    }
    inline bool ActorHasItem() const {
        return (uAttributes & ACTOR_HAS_ITEM) != 0;
    }
    inline bool ActorNearby() const {
        return (uAttributes & ACTOR_NEARBY) != 0;
    }

    static void _SelectTarget(unsigned int uActorID, int *a2,
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
    static void PlaySound(unsigned int uActorID, unsigned int uSoundID);
    static void Die(unsigned int uActorID);
    static void Resurrect(unsigned int uActorID);
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
                               int uSpellID, ABILITY_INDEX a4, unsigned int uSkillLevel);
    static void ActorDamageFromMonster(int attacker_id, unsigned int actor_id,
                                       Vec3i *pVelocity, ABILITY_INDEX a4);

    static unsigned short GetObjDescId(int spellId);

    static void AggroSurroundingPeasants(unsigned int uActorID, int a2);
    static bool ArePeasantsOfSameFaction(Actor *a1, Actor *a2);
    static void StealFrom(unsigned int uActorID);
    static void GiveItem(signed int uActorID, unsigned int uItemID,
                         unsigned int bGive);
    static void ToggleFlag(signed int uActorID, unsigned int uFlag,
                           int bToggle);
    static void ApplyFineForKillingPeasant(unsigned int uActorID);
    static void DrawHealthBar(Actor *actor, GUIWindow *window);
    int _43B3E0_CalcDamage(ABILITY_INDEX dmgSource);
    static void AddBloodsplatOnDamageOverlay(unsigned int uActorID, int a2,
                                             int a3);

    static void Arena_summon_actor(int monster_id, int x, int y, int z);
    static void DamageMonsterFromParty(int a1, unsigned int uActorID_Monster,
                                       Vec3i *pVelocity);
    static void MakeActorAIList_ODM();
    static int MakeActorAIList_BLV();
    static void UpdateActorAI();
    static void InitializeActors();
    static unsigned int SearchAliveActors(unsigned int *pTotalActors);
    static unsigned int SearchActorByMonsterID(unsigned int *pTotalActors,
                                               int uMonsterID);
    static unsigned int SearchActorByGroup(unsigned int *pTotalActors,
                                           unsigned int uGroup);
    static unsigned int SearchActorByID(unsigned int *pTotalActors,
                                        unsigned int a2);

    void LootActor();
    bool _427102_IsOkToCastSpell(enum SPELL_TYPE spell);
    ABILITY_INDEX special_ability_use_check(int a2);
    bool _4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4);
    bool ActorHitOrMiss(Player *pPlayer);
    int CalcMagicalDamageToActor(DAMAGE_TYPE dmgType, int incomingDmg);
    bool DoesDmgTypeDoDamage(DAMAGE_TYPE uType);

    int id = -1; // Actor index in pActors array.
    std::string pActorName;
    int16_t sNPC_ID = 0;
    int16_t field_22 = 0;
    unsigned int uAttributes = 0;
    int16_t sCurrentHP = 0;
    char field_2A[2] = {};
    MonsterInfo pMonsterInfo;
    int16_t word_000084_range_attack = 0;
    int16_t word_000086_some_monster_id = 0;  // base monster class monsterlist id
    uint16_t uActorRadius = 32;
    uint16_t uActorHeight = 128;
    uint16_t uMovementSpeed = 200;
    Vec3s vPosition;
    Vec3s vVelocity;
    uint16_t uYawAngle = 0;
    uint16_t uPitchAngle = 0;
    int16_t uSectorID = 0;
    uint16_t uCurrentActionLength = 0;
    Vec3s vInitialPosition;
    Vec3s vGuardingPosition;
    uint16_t uTetherDistance = 256;
    AIState uAIState = Standing;
    uint16_t uCurrentActionAnimation = ANIM_Standing; // TODO: use enum ActorAnimation
    uint16_t uCarriedItemID = 0;  // carried items are special items the
                                  // ncp carries (ie lute from bard)
    char field_B6 = 0;
    char field_B7 = 0;
    unsigned int uCurrentActionTime = 0;
    std::array<uint16_t, 8> pSpriteIDs = {{}};
    std::array<uint16_t, 4> pSoundSampleIDs = {{}};  // 1 die     3 bored
    IndexedArray<SpellBuff, ACTOR_BUFF_COUNT> pActorBuffs;
    std::array<ItemGen, 4> ActorHasItems;
    unsigned int uGroup = 0;
    unsigned int uAlly = 0;
    std::array<ActorJob, 8> pScheduledJobs;
    unsigned int uSummonerID = 0;
    unsigned int uLastCharacterIDToHit = 0;
    int dword_000334_unique_name = 0;
    char field_338[12] = {};
};
#pragma pack(pop)

// extern Actor pMonsterInfoUI_Doll;

extern std::vector<Actor> pActors;

bool CheckActors_proximity();
int IsActorAlive(unsigned int uType, unsigned int uParam,
                 unsigned int uNumAlive);  // idb
void sub_448518_npc_set_item(int npc, unsigned int item, int a3);
void ToggleActorGroupFlag(unsigned int uGroupID, unsigned int uFlag,
                          unsigned int bToggle);
bool Detect_Between_Objects(unsigned int uObjID, unsigned int uObj2ID);
bool SpawnActor(unsigned int uMonsterID);
void Spawn_Light_Elemental(int spell_power, int caster_skill_level,
                                     int duration_game_seconds);
void SpawnEncounter(struct MapInfo *pMapInfo, struct SpawnPointMM7 *spawn,
                    int a3, int a4, int a5);
void area_of_effect__damage_evaluate();
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
