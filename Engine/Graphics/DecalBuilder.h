#pragma once

#include "Engine/IocContainer.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/BSPModel.h"

using EngineIoc = Engine_::IocContainer;


// bloodsplats are created at enemy death as locations of where blood decal needs to be applied
#pragma pack(push, 1)
struct Bloodsplat {
    inline Bloodsplat() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
        this->radius = 0;
        this->dot_dist = 0;
        this->r = 0;
        this->g = 0;
        this->b = 0;
        this->blood_flags = 0;
        this->fade_timer = 0;
    }

    virtual ~Bloodsplat() {}

    float x;
    float y;
    float z;
    float radius;
    float dot_dist;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    int blood_flags;  // &1 for fade
    uint64_t fade_timer;
};
#pragma pack(pop)

// store for all the bloodsplats to be applied
#pragma pack(push, 1)
struct BloodsplatContainer {
    inline BloodsplatContainer() {
        uNumBloodsplats = 0;
    }

    virtual ~BloodsplatContainer() { }

    void AddBloodsplat(float x, float y, float z, float r, float g, float b,
        float radius, int a8, int a9);
    void AddBloodsplat(float x, float y, float z, float radius, unsigned char r,
        unsigned char g, unsigned char b);

    Bloodsplat pBloodsplats_to_apply[64];
    uint uNumBloodsplats;  // this loops round so old bloodsplats are replaced
};
#pragma pack(pop)


// decal is the created geometry to display
#pragma pack(push, 1)
struct Decal {
    void Decal_base_ctor();
    inline Decal() {
        Decal_base_ctor();
        decal_flags = 0;
    }
    double Fade_by_time();

    virtual ~Decal() {}

    int uNumVertices;
    RenderVertexSoft pVertices[64];
    int16_t DecalXPos;
    int16_t DecalYPos;
    int16_t DecalZPos;
    uint32_t uColorMultiplier;
    int DimmingLevel;

    uint64_t fadetime;
    int decal_flags;  // & 1 for fade
};
#pragma pack(pop)

// contains all of above
#pragma pack(push, 1)
struct DecalBuilder {
    DecalBuilder() {
        this->log = EngineIoc::ResolveLogger();
        this->bloodsplat_container = EngineIoc::ResolveBloodsplatContainer();

        this->DecalsCount = 0;
        /*for (unsigned int i = 0; i < 256; ++i) {
            this->pVertices[i].flt_2C = 0.0f;
        }*/
    }

    virtual ~DecalBuilder() {}

    void AddBloodsplat(float x, float y, float z, float r, float g, float b,
        float radius, int a8, int a9);
    void Reset(bool bPreserveBloodsplats);
    char ApplyDecals(int light_level, char a3, struct stru154* a4, int a5,
        struct RenderVertexSoft* a6,
        struct IndoorCameraD3D_Vec4* a7, char a8,
        unsigned int uSectorID);
    char _49B790_build_decal_geometry(int a2, char a3, Bloodsplat* a4, int64_t a5,
        float a6, unsigned int uColorMultiplier,
        float a8, struct stru314* a9,
        signed int a10,
        struct RenderVertexSoft* a11,
        char uClipFlags);
    bool ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID);
    char ApplyDecals_OutdoorFace(ODMFace* pFace);
    bool ApplyBloodSplatToTerrain(struct Polygon* a2, Vec3_float_* a3, float* a4,
        struct RenderVertexSoft* a5, unsigned int uStripType, char a7);
    void DrawDecals(float z_bias);
    void DrawBloodsplats();
    void DrawDecalDebugOutlines();

    Decal Decals[1024];  // actual decal geom store
    unsigned int DecalsCount;  // number of decals


    // for building decal geom
    int uNumDecals = 0;  // numeber of bloodsplats that overlap this face
    int std__vector_30B00C[1024]{};  // stores which ith element of blodsplats to apply outdoor bloodsplats/decals store for calc

    // sizes for building decal geometry
    float field_30C010 = 0;
    float field_30C014 = 0;
    float field_30C018 = 0;
    float field_30C01C = 0;
    float field_30C020 = 0;
    float field_30C024 = 0;
    float field_30C028 = 0;
    float field_30C02C = 0;
    float flt_30C030 = 0;
    float field_30C034 = 0;

    Log* log;
    BloodsplatContainer* bloodsplat_container;
};
#pragma pack(pop)
