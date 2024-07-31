#pragma once

#include <array>

struct ODMFace;
struct Span;
struct Edge;

struct Polygon {
    inline Polygon() { uNumVertices = 0; }

    // int _479295();
    // void _48276F_sr();
    // void _normalize_v_18();
    // void Create_48607B(SkyBillboardStruct *a2);

    inline bool IsWater() const { return this->flags & TILE_DESC_WATER; }
    inline bool IsWaterAnimDisabled() const {
        // TODO(captainurist) : is this check correct? We're checking that at least one of the flags is set.
        return this->flags & (TILE_DESC_SCROLL_DOWN | TILE_DESC_SCROLL_UP | TILE_DESC_SCROLL_LEFT | TILE_DESC_SCROLL_RIGHT);
    }

    float field_0 = 0;
    float field_4 = 0;
    float field_8 = 0;
    float field_C = 0;
    float field_10 = 0;
    unsigned int uNumVertices;
    int field_24 = 0;  // dot dist
    int sTextureDeltaU = 0;
    int sTextureDeltaV = 0;
    TILE_DESC_FLAGS flags = 0;
    int16_t field_32 = 0;
    int field_34 = 0;
    SkyBillboardStruct *ptr_38 = nullptr;
    GraphicsImage *texture = nullptr;  // struct Texture_MM7 *pTexture;
    Span *_unused_prolly_head = nullptr;
    Span *_unused_prolly_tail = nullptr;
    int **ptr_48 = nullptr;
    uint16_t uTileBitmapID = 0;
    int16_t field_4E = 0;
    int16_t pid = 0;
    int16_t field_52 = 0;
    ODMFace *pODMFace = nullptr;
    char dimming_level = 0;
    char field_59 = 0;
    char field_5A = 0;
    char field_5B = 0;
    char terrain_grid_z = 0;
    char terrain_grid_x = 0;
    uint8_t uBModelID = 0;
    uint8_t uBModelFaceID = 0;
    Edge *pEdgeList1[20] {};
    unsigned int uEdgeList1Size = 0;
    Edge *pEdgeList2[20] {};
    unsigned int uEdgeList2Size = 0;
    char field_108 = 0;
    char field_109 = 0;
    char field_10A = 0;
    char field_10B = 0;
};

extern std::array<struct Polygon, 2000 + 18000> array_77EC08;
