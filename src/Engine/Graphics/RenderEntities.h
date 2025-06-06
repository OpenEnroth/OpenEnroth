#pragma once

#include <cstdint>
#include <array>

#include "Engine/Pid.h"

#include "Library/Color/Color.h"
#include "Library/Geometry/Vec.h"

class Sprite;
class SpriteFrame;
class GraphicsImage;

struct RenderBillboard {
    float screenspace_projection_factor_x;
    float screenspace_projection_factor_y;
    float fov_x;
    float fov_y;
    int field_14_actor_id;
    Sprite *hwsprite;  // int16_t HwSpriteID;
    int16_t uPaletteIndex;
    int uIndoorSectorID;
    int16_t field_1E;  // flags
    int16_t world_x;
    int16_t world_y;
    int16_t world_z;
    int16_t screen_space_x;
    int16_t screen_space_y;
    int32_t screen_space_z;
    Pid object_pid;
    uint16_t dimming_level;
    Color sTintColor;
    SpriteFrame *pSpriteFrame;
};

/*   88 */
struct ODMRenderParams {
    ODMRenderParams() {
        this->shading_dist_shade = 0x800;
        shading_dist_shademist = 0x1000;
        this->bNoSky = 0;
        this->bDoNotRenderDecorations = 0;
        this->field_5C = 0;
        this->field_60 = 0;
        this->outdoor_no_wavy_water = 0;
        this->outdoor_no_mist = 0;
    }

    int shading_dist_shade;
    int shading_dist_shademist;
    int uNumPolygons = 0;
    unsigned int _unused_uNumEdges = 0;
    unsigned int _unused_uNumSurfs = 0;
    unsigned int _unused_uNumSpans = 0;
    unsigned int uNumBillboards = 0;
    float field_40 = 0;
    unsigned int bNoSky;
    unsigned int bDoNotRenderDecorations;
    int field_5C;
    int field_60;
    int outdoor_no_wavy_water;
    int outdoor_no_mist;
    int building_gamme = 0;
    int terrain_gamma = 0;
};
extern ODMRenderParams *pODMRenderParams;

struct RenderVertexSoft {
    Vec3f vWorldPosition {};
    Vec3f vWorldViewPosition {};
    float vWorldViewProjX = 0;
    float vWorldViewProjY = 0;
    float _rhw = 0;
    float u = 0;
    float v = 0;
    float flt_2C = 0;

    friend bool operator==(const RenderVertexSoft &l, const RenderVertexSoft &r) = default;
};

struct RenderVertexD3D3 {
    Vec3f pos {};
    float rhw = 0;
    Color diffuse;
    Color specular;
    Vec2f texcoord {};
};

struct RenderBillboardD3D {
    inline RenderBillboardD3D()
        : texture(nullptr),
          uNumVertices(4),
          z_order(0.f),
          opacity(Transparent),
          field_90(-1),
          screen_space_z(0),
          sParentBillboardID(-1),
          PaletteIndex(0) {}

    enum class OpacityType : uint32_t {
        Transparent = 0,
        Opaque_1 = 1,
        Opaque_2 = 2,
        Opaque_3 = 3,
        NoBlend = 0xFFFFFFFF
    };
    using enum OpacityType;

    GraphicsImage *texture;
    unsigned int uNumVertices;
    std::array<RenderVertexD3D3, 4> pQuads;
    float z_order;
    OpacityType opacity;
    int field_90;

    Pid object_pid;
    int screen_space_z;
    int sParentBillboardID;

    //int PaletteID;
    int PaletteIndex;
};

// TODO(pskelton): Simplify/remove/combine different billboard structs
struct SoftwareBillboard {
    void *pTarget;
    int screen_space_x;
    int screen_space_y;
    int screen_space_z;
    float screenspace_projection_factor_x;
    float screenspace_projection_factor_y;
    char field_18[8];
    uint16_t *pPalette;
    uint16_t *pPalette2;
    unsigned int uFlags;  // & 4   - mirror horizontally
    unsigned int uTargetPitch;
    unsigned int uViewportX;
    unsigned int uViewportY;
    unsigned int uViewportZ;
    unsigned int uViewportW;
    int field_44;
    int sParentBillboardID;
    Color sTintColor;
    Pid object_pid;
    int paletteID;
};
