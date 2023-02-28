#pragma once

#include <array>

#include "Utility/Flags.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/OurMath.h"

enum class ParticleFlag : uint32_t {
    ParticleType_Invalid = 0,
    ParticleType_Dropping = 0x0001,  // particle drops with time
    ParticleType_Rotating = 0x0004,  // particle rotates with time
    ParticleType_Ascending = 0x0008, // particle ascends with time
    ParticleType_Diffuse = 0x0100,   // colored plane
    ParticleType_Line = 0x0200,      // line
    ParticleType_Bitmap = 0x0400,    // textured planed
    ParticleType_Sprite = 0x0800
};
using enum ParticleFlag;
MM_DECLARE_FLAGS(ParticleFlags, ParticleFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(ParticleFlags)

// TODO(pskelton): eliminate this one
/*  305 */
#pragma pack(push, 1)
struct Particle_sw {
    ParticleFlags type;
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    unsigned int uDiffuse;
    int timeToLive;
    Texture *texture;  // unsigned int resource_id;// bitmap IDirect3DTexture
                       // idx or sprite idx depending on type
    int paletteID = 0;
    float particle_size;  // flt_28
    int field_2C;
    int field_30;
    int field_34;
    int field_38[12];
};
#pragma pack(pop)

/*  109 */
#pragma pack(push, 1)
struct Particle {
    ParticleFlags type = ParticleType_Invalid;
    float x = 0;
    float y = 0;
    float z = 0;
    float shift_x = 0;
    float shift_y = 0;
    float shift_z = 0;
    union {
        struct {
            unsigned char r, g, b, a;
        };
        unsigned int uParticleColor;
    };
    int timeToLive = 0;
    Texture *texture = nullptr;  // unsigned int resource_id;// bitmap IDirect3DTexture
                       // idx or sprite idx depending on type
    int paletteID = 0;
    float particle_size = 0;  // field_28
    float _x = 0;
    float _y = 0;
    float _z = 0;
    int rotation_speed = 0;
    int angle = 0;
    int uScreenSpaceX = 0;
    int uScreenSpaceY = 0;
    int uScreenSpaceZ = 0;  // line end x
    int uScreenSpaceW = 0;  // line end y
    union {
        // int sZValue;
        struct {
            unsigned short object_pid;
            short zbuffer_depth;
        };
    };
    int sZValue2 = 0;             // line end z
    float screenspace_scale = 1.0;  // fixed screenspace_scale {};  // int _screenspace_scale;
    float fov_x = 0;
    float fov_y = 0;
    int uLightColor_bgr = 0;
};
#pragma pack(pop)

/*  111 */
#pragma pack(push, 1)
struct stru2_LineList {
    unsigned int uNumLines = 0;
    RenderVertexD3D3 pLineVertices[48] {};
    char field_604[60] {};
};
#pragma pack(pop)

/*  110 */
#pragma pack(push, 1)
class ParticleEngine {
 public:
    ParticleEngine();

    static const int PARTICLES_ARRAY_SIZE = 500;

    void ResetParticles();
    void AddParticle(Particle_sw *patricle);
    void Draw();
    void UpdateParticles();
    bool ViewProject_TrueIfStillVisible_BLV(unsigned int uParticleID);
    void DrawParticles_BLV();

    std::array<Particle, PARTICLES_ARRAY_SIZE> pParticles;
    stru2_LineList pLines;
    char field_D160[4800]; // unused
    float field_E420; // unused
    int uStartParticle;
    int uEndParticle;
    int uTimeElapsed;
};
#pragma pack(pop)

/*  160 */
#pragma pack(push, 1)
struct TrailParticle {  // stru167
    inline TrailParticle() {
        x = 0;
        y = 0;
        z = 0;
        time_left = 0;
        time_to_live = 0;
        bgr16 = 0;
    }

    char field_0 = 0;
    char field_1 = 0;
    char field_2 = 0;
    char field_3 = 0;
    char field_4 = 0;
    char field_5 = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    int16_t time_left = 0;
    int16_t time_to_live = 0;
    char field_10 = 0;
    char field_11 = 0;
    char field_12 = 0;
    char field_13 = 0;
    int16_t bgr16 = 0;
    char field_16 = 0;
    char field_17 = 0;
};
#pragma pack(pop)

/*  363 */
#pragma pack(push, 1)
struct TrailParticleGenerator {  // stru167_wrap
 public:
    inline TrailParticleGenerator() { num_particles = 0; }

    void GenerateTrailParticles(int x, int y, int z, int bgr16);
    void UpdateParticles();

 protected:
    void AddParticle(int x, int y, int z, int bgr16);

    TrailParticle particles[100];
    int num_particles;
    int field_964 = 0;
};
#pragma pack(pop)
extern TrailParticleGenerator trail_particle_generator;  // 005118E8
