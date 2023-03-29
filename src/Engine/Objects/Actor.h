#pragma once

#include <vector>
#include <string>

#include "Engine/EngineIocContainer.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/Monsters.h"

#include "Utility/IndexedArray.h"

#include "ActorEnums.h"

class Actor;
struct SpawnPoint;

/*  357 */
#pragma pack(push, 1)
struct stru319 {
    inline stru319() {
        this->vis = EngineIocContainer::ResolveVis();
    }

    int which_player_to_attack(Actor *pActor);
    int _427546(int a2);
    int FindClosestActor(int pick_depth, int a3, int target_undead);

    char field_0 = 0;

    Vis *vis = nullptr;
};
#pragma pack(pop)

extern stru319 stru_50C198;  // idb

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
    uint16_t uAttributes = 0;
    uint8_t uAction = 0;
    uint8_t uHour = 0;
    uint8_t uDay = 0;
    uint8_t uMonth = 0;
};
#pragma pack(pop)

class GUIWindow;

/*   66 */
#pragma pack(push, 1)
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

    inline void ResetAnimation() { uAttributes &= ~ACTOR_ANIMATION; }
    inline void ResetQueue() { uAttributes &= ~ACTOR_STAND_IN_QUEUE; }
    inline void ResetActive() { uAttributes &= ~ACTOR_ACTIVE; }
    inline void ResetFullAiState() { uAttributes &= ~ACTOR_FULL_AI_STATE; }
    inline void ResetHasItem() { uAttributes &= ~ACTOR_HAS_ITEM; }
    inline void ResetHostile() { uAttributes &= ~ACTOR_HOSTILE; }
    inline void ResetAggressor() { uAttributes &= ~ACTOR_AGGRESSOR; }
    inline bool ActorEnemy() const {
        return uAttributes & ACTOR_AGGRESSOR;
    }
    inline bool ActorFriend() const {
        return !(uAttributes & ACTOR_AGGRESSOR);
    }
    inline bool ActorHasItem() const {
        return uAttributes & ACTOR_HAS_ITEM;
    }
    inline bool ActorNearby() const {
        return uAttributes & ACTOR_NEARBY;
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
    static void playSound(unsigned int uActorID, unsigned int uSoundID);
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
                               SPELL_TYPE uSpellID, ABILITY_INDEX a4, PLAYER_SKILL uSkill);
    static void ActorDamageFromMonster(int attacker_id, unsigned int actor_id,
                                       Vec3i *pVelocity, ABILITY_INDEX a4);

    static unsigned short GetObjDescId(SPELL_TYPE spellId);

    static void AggroSurroundingPeasants(unsigned int uActorID, int a2);
    static bool ArePeasantsOfSameFaction(Actor *a1, Actor *a2);
    static void StealFrom(unsigned int uActorID);
    static void GiveItem(signed int uActorID, ITEM_TYPE uItemID,
                         unsigned int bGive);
    static void ToggleFlag(signed int uActorID, ActorAttribute uFlag, bool bToggle);
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
    bool _427102_IsOkToCastSpell(SPELL_TYPE spell);
    ABILITY_INDEX special_ability_use_check(int a2);
    bool _4273BB_DoesHitOtherActor(Actor *defender, int a3, int a4);
    bool ActorHitOrMiss(Player *pPlayer);
    int CalcMagicalDamageToActor(DAMAGE_TYPE dmgType, int incomingDmg);
    bool DoesDmgTypeDoDamage(DAMAGE_TYPE uType);

    int id = -1; // Actor index in pActors array.
    std::string pActorName;
    int16_t sNPC_ID = 0;
    int16_t field_22 = 0;
    ActorAttributes uAttributes = 0;
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
    ActorAnimation uCurrentActionAnimation = ANIM_Standing;
    ITEM_TYPE uCarriedItemID = ITEM_NULL; // carried items are special items the
                                          // ncp carries (ie lute from bard)
    char field_B6 = 0;
    char field_B7 = 0;
    unsigned int uCurrentActionTime = 0;
    IndexedArray<uint16_t, ANIM_Count> pSpriteIDs = {{}};
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
    bool donebloodsplat{ false };
};
#pragma pack(pop)

// extern Actor pMonsterInfoUI_Doll;

extern std::vector<Actor> pActors;

bool CheckActors_proximity();
int IsActorAlive(unsigned int uType, unsigned int uParam,
                 unsigned int uNumAlive);  // idb
void sub_448518_npc_set_item(int npc, ITEM_TYPE item, int a3);
void ToggleActorGroupFlag(unsigned int uGroupID, ActorAttribute uFlag,
                          bool bValue);
bool Detect_Between_Objects(unsigned int uObjID, unsigned int uObj2ID);
bool SpawnActor(unsigned int uMonsterID);
void Spawn_Light_Elemental(int spell_power, PLAYER_SKILL_MASTERY caster_skill_mastery, int duration_game_seconds);
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
