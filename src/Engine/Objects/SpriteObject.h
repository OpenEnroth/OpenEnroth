#pragma once

#include <vector>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/SpriteObjectType.h"
#include "Engine/Objects/Actor.h"

class SpriteFrame;

/*   72 */
#pragma pack(push, 1)
struct SpriteObject {
    inline bool AttachedToActor() const {
        return uAttributes & SPRITE_ATTACHED_TO_HEAD;
    }

    int Create(int yaw, int pitch, int a4, int a5);
    void _46BEF1_apply_spells_aoe();
    void ExplosionTraps();
    unsigned int GetLifetime();
    SpriteFrame *GetSpriteFrame();
    bool IsUnpickable();
    bool HasSprite();
    uint8_t GetParticleTrailColorR();
    uint8_t GetParticleTrailColorG();
    uint8_t GetParticleTrailColorB();

    static void UpdateObject_fn0_BLV(unsigned int uLayingItemID);
    static void UpdateObject_fn0_ODM(unsigned int uLayingItemID);
    static void OnInteraction(unsigned int uLayingItemID);
    static bool Drop_Item_At(SPRITE_OBJECT_TYPE sprite, int x, int y,
                                      int z, int a4, int count, int a7,
                                      SPRITE_ATTRIBUTES attributes, ItemGen *a9);
    static void Create_Splash_Object(int x, int y, int z);
    static void InitializeSpriteObjects();

    SPRITE_OBJECT_TYPE uType = SPRITE_NULL;
    // unsigned __int16 uType;
    uint16_t uObjectDescID = 0; // Zero means free slot, can reuse.
    Vec3i vPosition;
    Vec3s vVelocity;
    uint16_t uFacing = 0;
    uint16_t uSoundID = 0;
    SPRITE_ATTRIBUTES uAttributes;
    int16_t uSectorID = 0;
    uint16_t uSpriteFrameID = 0;
    int16_t field_20 = 0;
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
    Vec3i field_64;  // starting position
};
#pragma pack(pop)

void CompactLayingItemsList();

extern std::vector<SpriteObject> pSpriteObjects;

bool _46BFFA_update_spell_fx(unsigned int uLayingItemID, signed int pid);
void Apply_Spell_Sprite_Damage(unsigned int uLayingItemID, signed int pid);  // idb
unsigned int sub_46DEF2(signed int pid, unsigned int uLayingItemID);
