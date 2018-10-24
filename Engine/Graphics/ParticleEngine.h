#pragma once
#include "Engine/Graphics/IRender.h"
#include "Engine/OurMath.h"

enum ParticleType : uint32_t {
    ParticleType_Invalid = 0,
    ParticleType_1 = 0x0001,
    ParticleType_Rotating = 0x0004,
    ParticleType_8 = 0x0008,
    ParticleType_Diffuse = 0x0100,  // colored plane
    ParticleType_Line = 0x0200,     // line
    ParticleType_Bitmap = 0x0400,   // textured planed
    ParticleType_Sprite = 0x0800
};

/*  305 */
#pragma pack(push, 1)
struct Particle_sw {
    unsigned int type;
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
    unsigned int type;
    float x;
    float y;
    float z;
    float flt_10;
    float flt_14;
    float flt_18;
    union {
        struct {
            unsigned char r, g, b, a;
        };
        unsigned int uParticleColor;
    };
    int timeToLive;
    Texture *texture;  // unsigned int resource_id;// bitmap IDirect3DTexture
                       // idx or sprite idx depending on type
    float particle_size;  // field_28
    float _x;
    float _y;
    float _z;
    int rotation_speed;
    int angle;
    int uScreenSpaceX;
    int uScreenSpaceY;
    int uScreenSpaceZ;  // line end x
    int uScreenSpaceW;  // line end y
    union {
        // int sZValue;
        struct {
            unsigned short object_pid;
            short zbuffer_depth;
        };
    };
    int sZValue2;             // line end z
    fixed screenspace_scale;  // int _screenspace_scale;
    float fov_x;
    float fov_y;
    int uLightColor_bgr;
};
#pragma pack(pop)

/*  111 */
#pragma pack(push, 1)
struct stru2_LineList {
    unsigned int uNumLines;
    RenderVertexD3D3 pLineVertices[48];
    char field_604[60];
};
#pragma pack(pop)

/*  110 */
#pragma pack(push, 1)
class ParticleEngine {
 public:
    ParticleEngine();

    void ResetParticles();
    void AddParticle(Particle_sw *a2);
    void Draw();
    void UpdateParticles();
    bool ViewProject_TrueIfStillVisible_BLV(unsigned int uParticleID);
    void DrawParticles_BLV();

    Particle pParticles[500];
    stru2_LineList pLines;
    char field_D160[4800];
    float field_E420;
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

    char field_0;
    char field_1;
    char field_2;
    char field_3;
    char field_4;
    char field_5;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t time_left;
    int16_t time_to_live;
    char field_10;
    char field_11;
    char field_12;
    char field_13;
    int16_t bgr16;
    char field_16;
    char field_17;
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
    int field_964;
};
#pragma pack(pop)
extern TrailParticleGenerator trail_particle_generator;  // 005118E8
