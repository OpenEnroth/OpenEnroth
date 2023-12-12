#pragma once

#include <array>

#include "Engine/Graphics/RenderEntities.h"
#include "Engine/Time/Duration.h"

#include "Library/Color/Color.h"

#include "Utility/Flags.h"

class GraphicsImage;

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
struct Particle_sw {
    ParticleFlags type{ ParticleType_Invalid };
    float x{};
    float y{};
    float z{};
    float r{};
    float g{};
    float b{};
    Color uDiffuse{};
    Duration timeToLive{};
    GraphicsImage *texture{ nullptr };
    int paletteID{ 0 };
    float particle_size{};
    int field_2C{};
    int field_30{};
    int field_34{};
    int field_38[12]{};
};

struct Particle {
    ParticleFlags type = ParticleType_Invalid;
    float x = 0;
    float y = 0;
    float z = 0;
    float shift_x = 0;
    float shift_y = 0;
    float shift_z = 0;
    Color uParticleColor;
    Duration timeToLive;
    GraphicsImage *texture = nullptr;  // unsigned int resource_id;// bitmap IDirect3DTexture
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
    short zbuffer_depth;
    int sZValue2 = 0;             // line end z
    float screenspace_scale = 1.0;  // fixed screenspace_scale {};  // int _screenspace_scale;
    float fov_x = 0;
    Color uLightColor_bgr;
};

struct stru2_LineList {
    unsigned int uNumLines = 0;
    RenderVertexD3D3 pLineVertices[48] {};
    char field_604[60] {};
};

class ParticleEngine {
 public:
    static const int PARTICLES_ARRAY_SIZE = 500;

    /**
     * Particle engine constructor.
     *
     * @offset 0x48AAC5
     */
    ParticleEngine();

    /**
     * Remove all active particles if any and initialize/reinitialize then particles engine.
     *
     * @offset 0x48AAF6
     */
    void ResetParticles();

    /**
     * Add particle to engine.
     *
     * @offset 0x48AB23
     */
    void AddParticle(Particle_sw *particle);

    /**
     * Draw all active particles.
     *
     * @offset 0x48ABF3
     */
    void Draw();

    /**
     * Update all active particles based on time elapsed since previous tick.
     * Transform particles if needed and remove them if particle live time expired.
     *
     * @offset 0x48AC65
     */
    void UpdateParticles();

    /**
     * @offset 0x48AE74
     */
    bool ViewProject_TrueIfStillVisible_BLV(unsigned int uParticleID);

    /**
     * @offset 0x48BBA6
     */
    void DrawParticles_BLV();

    std::array<Particle, PARTICLES_ARRAY_SIZE> pParticles;
    stru2_LineList pLines;
    char field_D160[4800]; // unused
    float field_E420; // unused
    int uStartParticle;
    int uEndParticle;
    Duration uTimeElapsed;
};

struct TrailParticle {
    char field_0 = 0;
    char field_1 = 0;
    char field_2 = 0;
    char field_3 = 0;
    char field_4 = 0;
    char field_5 = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    Duration time_left;
    Duration time_to_live;
    char field_10 = 0;
    char field_11 = 0;
    char field_12 = 0;
    char field_13 = 0;
    Color color;
    char field_16 = 0;
    char field_17 = 0;
};

struct TrailParticleGenerator {  // stru167_wrap
 public:
    inline TrailParticleGenerator() { num_particles = 0; }

    void GenerateTrailParticles(int x, int y, int z, Color color);
    void UpdateParticles();

 protected:
    void AddParticle(int x, int y, int z, Color color);

    TrailParticle particles[100];
    int num_particles;
    int field_964 = 0;
};

extern TrailParticleGenerator trail_particle_generator;  // 005118E8
