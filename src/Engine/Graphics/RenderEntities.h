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
    float rhw = 0.0f;
    Color diffuse;
    Vec2f texcoord;
};

struct RenderBillboard {
    Vec2f screenspace_projection_factor;
    Sprite* hwsprite;
    int16_t uPaletteId;
    int uIndoorSectorID;
    BillboardFlags flags;
    Vec3f worldPos;
    Vec2f screenPos;
    int32_t view_space_z;
    float view_space_L2 = 0.0f;
    Pid object_pid;
    uint16_t dimming_level;
    Color sTintColor;
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
    float view_space_L2 = 0.0f;
};
