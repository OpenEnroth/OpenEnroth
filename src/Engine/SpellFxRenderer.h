#pragma once

#include <array>
#include <memory>
#include <cstdint>

#include "Engine/Spells/SpellEnums.h"
#include "Engine/Time/Duration.h"

#include "Library/Color/ColorTable.h"

#include "Library/Geometry/Vec.h"

class Actor;
class GraphicsImage;
class ParticleEngine;
struct SpriteObject;

extern std::array<Vec3f, 66> sphereVertPos;
extern std::array<int32_t, 128 * 3> sphereVertInd;

struct SpellFX_Billboard {
    /**
     * @offset 0x4775B1
     */
    inline SpellFX_Billboard() {
        uNumVertices = 0;
    }

    /**
     * @offset 0x4775C4
     */
    virtual ~SpellFX_Billboard();

    int SpellFXNearClipAdjust(float a2);
    int SpellFXFarClipAdjust(float a2);
    int SpellFXViewTransform();
    bool SpellFXViewClip();
    int SpellFXProject();
    void _47829F_sphere_particle(Vec3f center, float scale, Color diffuse);

    // billboard quad vertex
    struct local_01 {
        Vec3f pos;
        Color diffuse;
    };

    // spellfx verts
    int uNumVertices;
    local_01 field_14[5]{};  // world coords of verts
    local_01 field_64[5]{};  // view coords
    local_01 field_B4[5]{};  // clipped view coords
    local_01 field_104[5]{};  // projected billboard coords
};

struct PlayerBuffAnim {
    int16_t bRender = false;
    int16_t field_2 = 0;
    Duration uSpellAnimTimeElapsed;
    Duration uSpellAnimTime;
    int uSpellIconID = 0;
};

struct ProjectileAnim {
    float srcX;
    float srcY;
    float srcZ;
    float dstX;
    float dstY;
    float dstZ;
    GraphicsImage *texture;  // int uTextureID;
};

struct stru6_stru2 {
    // int field_0;
    float flt_0_x;
    float flt_4_y;
    float flt_8_z;
    int field_C;
};

struct SpellFxRenderer {
    /**
     * @offset 0x4A7155
     */
    explicit inline SpellFxRenderer(std::shared_ptr<ParticleEngine> particle_engine) {
        this->particle_engine = particle_engine;

        this->field_204 = 0;
        this->uFadeTime = 0_ticks;
        this->uNumProjectiles = 0;
        this->field_0 = 0;
        this->uAnimLength = 0_ticks;

        _spellFXSphereInstance = new SpellFX_Billboard();
        //_spellFXSphereInstance->Initialize(colorTable.OrangeyRed);
    }

    /**
     * @offset 0x4A71DC
     */
    ~SpellFxRenderer() {
        delete _spellFXSphereInstance;
        _spellFXSphereInstance = nullptr;
    }

    void DoAddProjectile(float srcX, float srcY, float srcZ, float dstX,
                         float dstY, float dstZ, GraphicsImage *);
    void DrawProjectiles();
    void _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
        SpriteObject *a2, Color uDiffuse, GraphicsImage *texture);
    void _4A75CC_single_spell_collision_particle(SpriteObject *a1,
                                                 Color uDiffuse,
                                                 GraphicsImage *texture);
    void _4A7688_fireball_collision_particle(SpriteObject *a2);
    void _4A77FD_implosion_particle_d3d(SpriteObject *a1);
    void _4A7948_mind_blast_after_effect(SpriteObject *a1);
    bool AddMobileLight(SpriteObject *a1, Color uDiffuse,
                        int uRadius);
    void
    _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
        SpriteObject *a1, Color uDiffuse, GraphicsImage *texture, float a4);
    void _4A7C07_stun_spell_fx(SpriteObject *a2);
    void AddProjectile(SpriteObject *a2, int a3, GraphicsImage *);
    /**
     * @offset 0x4A7E89
     */
    void sparklesOnActorAfterItCastsBuff(Actor *pActor, Color uDiffuse);
    void _4A7F74(int x, int y, int z);
    float _4A806F_get_mass_distortion_value(Actor *pActor);
    // void _4A80DC_implosion_particle_sw(SpriteObject *a2);
    bool RenderAsSprite(SpriteObject *a2);
    void SetPlayerBuffAnim(SpellId uSpellID, uint16_t uPlayerID);
    void SetPartyBuffAnim(SpellId uSpellID);
    void FadeScreen__like_Turn_Undead_and_mb_Armageddon(Color uDiffuseColor, Duration uFadeTime);
    void _4A8BFC_prismatic_light();
    void RenderSpecialEffects();
    void DrawPlayerBuffAnims();
    void LoadAnimations();

    int field_0;  // count of have many stored in array_4
    stru6_stru2 array_4[32];  // stores source position

    int field_204;  // fireball sphere tally

    std::array<PlayerBuffAnim, 4> pCharacterBuffs;
    std::array<ProjectileAnim, 32> pProjectiles;
    int uNumProjectiles;
    SpellFX_Billboard *_spellFXSphereInstance;
    int field_5D0;
    Duration uAnimLength;
    Duration uFadeTime;
    Duration uFadeLength;
    Color uFadeColor;
    GraphicsImage *effpar01;  // unsigned int effpar01; // trail fire
    GraphicsImage *effpar02;  // unsigned int effpar02;
    GraphicsImage *effpar03;  // unsigned int effpar03; // trail particle
    unsigned int _unused_uSpriteID_sp57c;
    int field_5F4;

    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
};
