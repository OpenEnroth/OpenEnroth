#pragma once

#include <vector>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/SpriteObjectType.h"
#include "Engine/Objects/Actor.h"

class SpriteFrame;

/*   72 */
#pragma pack(push, 1)
struct SpriteObject {
    inline bool attachedToActor() const {
        return uAttributes & SPRITE_ATTACHED_TO_HEAD;
    }

    int Create(int yaw, int pitch, int a4, int a5);

    /**
     * @offset 0x46BEF1
     */
    bool applyShrinkRayAoe();
    void explosionTraps();
    unsigned int GetLifetime();
    SpriteFrame *GetSpriteFrame();
    bool IsUnpickable();
    bool HasSprite();
    uint8_t GetParticleTrailColorR();
    uint8_t GetParticleTrailColorG();
    uint8_t GetParticleTrailColorB();

    inline void spellSpriteStop() {
        uSpriteFrameID = 0;
        vVelocity = Vec3s(0.0, 0.0, 0.0);
    }

    /**
     * @offset 0x47136C
     */
    static void updateObjectBLV(unsigned int uLayingItemID);

    /**
     * @offset 0x471C03
     */
    static void updateObjectODM(unsigned int uLayingItemID);
    static void OnInteraction(unsigned int uLayingItemID);
    /**
     * Create sprite(s).
     *
     * @param spriteType     Type of sprite to drop.
     * @param pos            Position of sprite.
     * @param speed          Speed of sprite.
     * @param count          Number of sprites to drop.
     * @param randomRotate   Randomize direction vector of sprite (if false drop will be vertical).
     * @param attributes     Sprite attributes.
     * @param item           Containing item of sprite (may be null).
     * @offset 0x42F7EB
     */
    static bool dropItemAt(SPRITE_OBJECT_TYPE spriteType, Vec3i pos, int speed, int count = 1,
                           bool randomRotate = false, SPRITE_ATTRIBUTES attributes = 0, ItemGen *item = nullptr);
    static void createSplashObject(Vec3i pos);
    static void InitializeSpriteObjects();

    SPRITE_OBJECT_TYPE uType = SPRITE_NULL;
    // unsigned __int16 uType;
    uint16_t uObjectDescID = 0; // Zero means free slot, can reuse.
    Vec3i vPosition;
    Vec3s vVelocity;
    uint16_t uFacing = 0;
    uint16_t uSoundID = 0;
    SPRITE_ATTRIBUTES uAttributes = 0;
    int16_t uSectorID = 0;
    uint16_t uSpriteFrameID = 0;
    int16_t tempLifetime = 0;
    int16_t field_22_glow_radius_multiplier = 1;
    ItemGen containing_item;
    SPELL_TYPE uSpellID = SPELL_NONE;
    int spell_level = 0;
    PLAYER_SKILL_MASTERY spell_skill = PLAYER_SKILL_MASTERY_NONE;
    int field_54 = 0;
    int spell_caster_pid = 0;
    int spell_target_pid = 0;
    char field_60_distance_related_prolly_lod = 0;
    ABILITY_INDEX field_61 = ABILITY_ATTACK1;
    char field_62[2] = {};
    Vec3i initialPosition;
    int _lastParticleTime{};
    int _ticksPerParticle{ 2 }; // how many 1/128ths between particles
};
#pragma pack(pop)

void CompactLayingItemsList();

extern std::vector<SpriteObject> pSpriteObjects;

/**
 * @offset 0x46BFFA
 */
bool processSpellImpact(unsigned int uLayingItemID, signed int pid);

/**
 * @offset 0x43A97E
 */
void applySpellSpriteDamage(unsigned int uLayingItemID, signed int pid);  // idb

/**
 * @offset 0x46DEF2
 */
unsigned int collideWithActor(unsigned int uLayingItemID, signed int pid);
