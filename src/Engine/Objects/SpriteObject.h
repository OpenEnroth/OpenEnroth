#pragma once

#include <vector>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/SpriteObjectType.h"
#include "Engine/Objects/Actor.h"
#include "Engine/VectorTypes.h"

class SpriteFrame;

/*   72 */
#pragma pack(push, 1)
struct SpriteObject {
    inline bool AttachedToActor() const {
        return uAttributes & SPRITE_ATTACHED_TO_HEAD;
    }

    SpriteObject();
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

    SPRITE_OBJECT_TYPE uType;
    // unsigned __int16 uType;
    unsigned __int16 uObjectDescID = 0; // Zero means free slot, can reuse.
    Vec3i vPosition;
    Vec3s vVelocity;
    unsigned __int16 uFacing;
    unsigned __int16 uSoundID;
    SPRITE_ATTRIBUTES uAttributes;
    __int16 uSectorID;
    unsigned __int16 uSpriteFrameID;
    __int16 field_20;
    __int16 field_22_glow_radius_multiplier;
    struct ItemGen containing_item;
    int spell_id;
    int spell_level;
    int spell_skill;
    int field_54;
    int spell_caster_pid;
    int spell_target_pid;
    char field_60_distance_related_prolly_lod;
    ABILITY_INDEX field_61;
    char field_62[2]{};
    Vec3i field_64;  // starting position
};
#pragma pack(pop)

void CompactLayingItemsList();

extern std::vector<SpriteObject> pSpriteObjects;

bool _46BFFA_update_spell_fx(unsigned int uLayingItemID, signed int pid);
void Apply_Spell_Sprite_Damage(unsigned int uLayingItemID, signed int pid);  // idb
unsigned int sub_46DEF2(signed int pid, unsigned int uLayingItemID);
