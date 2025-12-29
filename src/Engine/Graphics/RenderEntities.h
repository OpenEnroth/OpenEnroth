#pragma once

#include <cstdint>
#include <array>

#include "Engine/Pid.h"

#include "Library/Color/Color.h"
#include "Library/Geometry/Vec.h"

#include "SpriteEnums.h"

class Sprite;
class SpriteFrame;
class GraphicsImage;

struct ODMRenderParams {
    int shading_dist_shade = 0x800;
    int shading_dist_shademist = 0x1000;
    int uNumPolygons = 0;
    unsigned int _unused_uNumEdges = 0;
    unsigned int _unused_uNumSurfs = 0;
    unsigned int _unused_uNumSpans = 0;
    unsigned int uNumBillboards = 0;
    float field_40 = 0;
    unsigned int bNoSky = 0;
    unsigned int bDoNotRenderDecorations = 0;
    int field_5C = 0;
    int field_60 = 0;
    int outdoor_no_wavy_water = 0;
    int outdoor_no_mist = 0;
    int building_gamme = 0;
    int terrain_gamma = 0;
};
extern ODMRenderParams *pODMRenderParams;

struct RenderVertexSoft {
    Vec3f vWorldPosition;
    Vec3f vWorldViewPosition;
    Vec2f vWorldViewProj;
    float _rhw = 0;
    float u = 0;
    float v = 0;
    float flt_2C = 0;

    friend bool operator==(const RenderVertexSoft &l, const RenderVertexSoft &r) = default;
};

struct RenderVertexD3D3 {
    Vec3f pos;
    float rhw = 0;
    Color diffuse;
    Color specular;
    Vec2f texcoord;
};

struct RenderBillboard {
    float screenspace_projection_factor_x;
    float screenspace_projection_factor_y;
    float fov_x;
    float fov_y;
    int field_14_actor_id;
    Sprite* hwsprite;  // int16_t HwSpriteID;
    int16_t uPaletteId;
    int uIndoorSectorID;
    BillboardFlags flags;  // flags
    int16_t world_x;
    int16_t world_y;
    int16_t world_z;
    int16_t screen_space_x;
    int16_t screen_space_y;
    int32_t view_space_z;
    Pid object_pid;
    uint16_t dimming_level;
    Color sTintColor;
    SpriteFrame* pSpriteFrame;
};

struct RenderBillboardD3D {
    enum class OpacityType : uint32_t {
        Transparent = 0,
        Opaque_1 = 1,
        Opaque_2 = 2,
        Opaque_3 = 3,
        NoBlend = 0xFFFFFFFF
    };
    using enum OpacityType;

    GraphicsImage *texture = nullptr;
    unsigned int uNumVertices = 4;
    std::array<RenderVertexD3D3, 4> pQuads;
    OpacityType opacity = Transparent;

    int view_space_z = 0;
    int sParentBillboardID = -1;
    int paletteId = 0;
};
