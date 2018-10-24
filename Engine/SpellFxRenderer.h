#pragma once

#include <cstdint>

#include "Engine/IocContainer.h"

using EngineIoc = Engine_::IocContainer;

class Texture;

/*  120 */
#pragma pack(push, 1)
struct stru6_stru1_indoor_sw_billboard {
    //----- (004775B1) --------------------------------------------------------
    inline stru6_stru1_indoor_sw_billboard() {
        uNumVertices = 0;
        uNumVec4sInArray1 = 0;
        uNumVec3sInArray2 = 0;
        pArray1 = nullptr;
        pArray2 = nullptr;
    }

    //----- (004775C4) --------------------------------------------------------
    virtual ~stru6_stru1_indoor_sw_billboard();

    int _4775ED(float a2);
    int _477927(float a2);
    int sub_477C61();
    bool sub_477F63();
    int sub_47802A();
    void Initialize(int a2);
    void _47829F_sphere_particle(float x_offset, float y_offset, float z_offset,
                                 float scale, int diffuse);

    struct local_01 {
        float x;
        float y;
        float z;
        int diffuse;
    };

    unsigned int uNumVec4sInArray1;
    struct stru16x *pArray1;
    unsigned int uNumVec3sInArray2;
    struct stru160 *pArray2;
    int uNumVertices;
    local_01 field_14[5];  // world coords of verts
    int field_64[20];
    float field_B4[20];
    local_01 field_104[5];  // billboard coords
};
#pragma pack(pop)

/*  122 */
#pragma pack(push, 1)
struct PlayerBuffAnim {
    inline PlayerBuffAnim()
        : bRender(false), uSpellAnimTime(0), uSpellIconID(0) {}

    int16_t bRender;
    int16_t field_2;
    int uSpellAnimTimeElapsed;
    int uSpellAnimTime;
    int uSpellIconID;
};
#pragma pack(pop)

/*  307 */
#pragma pack(push, 1)
struct ProjectileAnim {
    float srcX;
    float srcY;
    float srcZ;
    float dstX;
    float dstY;
    float dstZ;
    Texture *texture;  // int uTextureID;
};
#pragma pack(pop)

/*  306 */
#pragma pack(push, 1)
struct stru6_stru2 {
    // int field_0;
    float flt_0_x;
    float flt_4_y;
    float flt_8_z;
    int field_C;
};
#pragma pack(pop)

/* stru6 121 */
#pragma pack(push, 1)
struct SpellFxRenderer {
    //----- (004A7155) --------------------------------------------------------
    inline SpellFxRenderer() {
        this->particle_engine = EngineIoc::ResolveParticleEngine();

        this->field_204 = 0;
        this->uFadeTime = 0;
        this->uNumProjectiles = 0;
        this->field_0 = 0;
        this->uAnimLength = 0;

        pStru1 = new stru6_stru1_indoor_sw_billboard;
        pStru1->Initialize(0xFF3C1Eu);
    }
    //----- (004A71DC) --------------------------------------------------------
    ~SpellFxRenderer() {
        delete pStru1;
        pStru1 = nullptr;
    }

    void DoAddProjectile(float srcX, float srcY, float srcZ, float dstX,
                         float dstY, float dstZ, Texture *);
    void DrawProjectiles();
    void _4A73AA_hanging_trace_particles___like_fire_strike_ice_blast_etc(
        struct SpriteObject *a2, unsigned int uDiffuse, Texture *texture);
    void _4A75CC_single_spell_collision_particle(struct SpriteObject *a1,
                                                 unsigned int uDiffuse,
                                                 Texture *texture);
    void _4A7688_fireball_collision_particle(struct SpriteObject *a2);
    void _4A77FD_implosion_particle_d3d(struct SpriteObject *a1);
    void _4A78AE_sparks_spell(struct SpriteObject *a1);
    void _4A7948_mind_blast_after_effect(struct SpriteObject *a1);
    bool AddMobileLight(struct SpriteObject *a1, unsigned int uDiffuse,
                        int uRadius);
    void
    _4A7A66_miltiple_spell_collision_partifles___like_after_sparks_or_lightning(
        SpriteObject *a1, unsigned int uDiffuse, Texture *texture, float a4);
    void _4A7C07_stun_spell_fx(struct SpriteObject *a2);
    void AddProjectile(struct SpriteObject *a2, int a3, Texture *);
    void _4A7E89_sparkles_on_actor_after_it_casts_buff(struct Actor *pActor,
                                                       unsigned int uDiffuse);
    void _4A7F74(int x, int y, int z);
    float _4A806F_get_mass_distortion_value(struct Actor *pActor);
    // void _4A80DC_implosion_particle_sw(struct SpriteObject *a2);
    bool RenderAsSprite(struct SpriteObject *a2);
    void SetPlayerBuffAnim(uint16_t uSpellID, uint16_t uPlayerID);
    void FadeScreen__like_Turn_Undead_and_mb_Armageddon(
        unsigned int uDiffuseColor, unsigned int uFadeTime);
    int _4A8BFC();
    void RenderSpecialEffects();
    void DrawPlayerBuffAnims();
    void LoadAnimations();

    int field_0;
    stru6_stru2 array_4[31];
    int field_1F4;
    int field_1F8;
    int field_1FC;
    int field_200;
    int field_204;
    PlayerBuffAnim pPlayerBuffs[4];
    ProjectileAnim pProjectiles[32];
    int uNumProjectiles;
    stru6_stru1_indoor_sw_billboard *pStru1;
    int field_5D0;
    int uAnimLength;
    int uFadeTime;
    int uFadeLength;
    int uFadeColor;
    Texture *effpar01;  // unsigned int effpar01; // trail fire
    Texture *effpar02;  // unsigned int effpar02;
    Texture *effpar03;  // unsigned int effpar03; // trail particle
    unsigned int _unused_uSpriteID_sp57c;
    int field_5F4;

    ParticleEngine *particle_engine = nullptr;
};
#pragma pack(pop)
