#pragma once
#include <array>


class Texture;
struct ODMFace;
struct Span;
struct Edge;

/*  141 */
// stru148
#pragma pack(push, 1)
struct Polygon {
    inline Polygon() { uNumVertices = 0; }

    // int _479295();
    // void _48276F_sr();
    // void _normalize_v_18();
    // void Create_48607B(struct SkyBillboardStruct *a2);

    inline bool IsWater() const { return this->flags & 2; }
    inline bool IsWaterAnimDisabled() const { return this->flags & 0x3C00; }

    float field_0 = 0;
    float field_4 = 0;
    float field_8 = 0;
    float field_C = 0;
    float field_10 = 0;
    unsigned int uNumVertices;
    Vec3i v_18;  // fp pitch rotation vec
    int field_24 = 0;  // dot dist
    int sTextureDeltaU = 0;
    int sTextureDeltaV = 0;
    int16_t flags = 0;  // &2 -> Water  &3C00 still water (no anim)
    int16_t field_32 = 0;
    int field_34 = 0;
    struct SkyBillboardStruct *ptr_38 = nullptr;
    Texture *texture = nullptr;  // struct Texture_MM7 *pTexture;
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
#pragma pack(pop)

extern std::array<struct Polygon, 2000 + 18000> array_77EC08;
