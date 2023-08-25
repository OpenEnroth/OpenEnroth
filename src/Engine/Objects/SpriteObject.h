#pragma once

#include <vector>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/SpriteObjectEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Pid.h"

#include "Library/Color/Color.h"

#include "Utility/Geometry/Vec.h"

class SpriteFrame;

/*   72 */
struct SpriteObject {
    inline bool attachedToActor() const {
        return uAttributes & SPRITE_ATTACHED_TO_HEAD;
    }

    // WARNING - this function should not be called for any item in pSpriteObjects vector
    int Create(int yaw, int pitch, int a4, int a5);

    /**
     * @offset 0x46BEF1
     */
    bool applyShrinkRayAoe();
    void explosionTraps();
    unsigned int GetLifetime();
    SpriteFrame *getSpriteFrame();
    bool IsUnpickable();
    bool HasSprite();
    Color GetParticleTrailColor();

    inline void spellSpriteStop() {
        uSpriteFrameID = 0;
        vVelocity = Vec3i(0, 0, 0);
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
                           bool randomRotate = false, SpriteAttributes attributes = 0, ItemGen *item = nullptr);
    static void createSplashObject(Vec3i pos);
    static void InitializeSpriteObjects();

    SPRITE_OBJECT_TYPE uType = SPRITE_NULL;
    // unsigned __int16 uType;
    uint16_t uObjectDescID = 0; // Index into pObjectList->pObjects. Zero means free slot, can reuse.
    Vec3i vPosition;
    Vec3i vVelocity;
    uint16_t uFacing = 0;
    uint16_t uSoundID = 0;
    SpriteAttributes uAttributes = 0;
    int uSectorID = 0;
    uint16_t uSpriteFrameID = 0;
    int16_t tempLifetime = 0;
    int16_t field_22_glow_radius_multiplier = 1;
    ItemGen containing_item;
    SPELL_TYPE uSpellID = SPELL_NONE;
    int spell_level = 0;
    CharacterSkillMastery spell_skill = CHARACTER_SKILL_MASTERY_NONE;
    int field_54 = 0;
    Pid spell_caster_pid;
    Pid spell_target_pid;
    char field_60_distance_related_prolly_lod = 0;
    ABILITY_INDEX spellCasterAbility = ABILITY_ATTACK1;
    Vec3i initialPosition;
    int _lastParticleTime{};
    int _ticksPerParticle{ 2 }; // how many 1/128ths between particles
};

void CompactLayingItemsList();

extern std::vector<SpriteObject> pSpriteObjects;

/**
 * @offset 0x46BFFA
 */
bool processSpellImpact(unsigned int uLayingItemID, Pid pid);

/**
 * @offset 0x43A97E
 */
void applySpellSpriteDamage(unsigned int uLayingItemID, Pid pid);  // idb

/**
 * @offset 0x46DEF2
 */
unsigned int collideWithActor(unsigned int uLayingItemID, Pid pid);
