#pragma once
#include <array>

#include "Engine/VectorTypes.h"


class Texture;
struct ODMFace;
struct Span;
struct Edge;

/*  141 */
// stru148
#pragma pack(push, 1)
struct Polygon
{
    inline Polygon()
    {
        uNumVertices = 0;
    }

    int _479295();
    //void _48276F_sr();
    void _normalize_v_18();
    void Create_48607B(struct stru149 *a2);

    inline bool IsWater() const { return this->flags & 2; }
    inline bool IsWaterAnimDisabled() const { return this->flags & 0x3C00; }

    float field_0;
    float field_4;
    float field_8;
    float field_C;
    float field_10;
    unsigned int uNumVertices;
    Vec3_int_ v_18;
    int field_24;
    int sTextureDeltaU;
    int sTextureDeltaV;
    __int16 flags; // &2 -> Water  &3C00 still water (no anim)
    __int16 field_32;
    int field_34;
    struct stru149 *ptr_38;
    Texture *texture;//struct Texture_MM7 *pTexture;
    Span *_unused_prolly_head;
    Span *_unused_prolly_tail;
    int **ptr_48;
    unsigned __int16 uTileBitmapID;
    __int16 field_4E;
    __int16 pid;
    __int16 field_52;
    ODMFace *pODMFace;
    char dimming_level;
    char field_59;
    char field_5A;
    char field_5B;
    char terrain_grid_z;
    char terrain_grid_x;
    unsigned __int8 uBModelID;
    unsigned __int8 uBModelFaceID;
    Edge *pEdgeList1[20];
    unsigned int uEdgeList1Size;
    Edge *pEdgeList2[20];
    unsigned int uEdgeList2Size;
    char field_108;
    char field_109;
    char field_10A;
    char field_10B;
};
#pragma pack(pop)

extern std::array<struct Polygon, 2000 + 18000> array_77EC08;