#pragma once

#include "Engine/IocContainer.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/BSPModel.h"

using EngineIoc = Engine_::IocContainer;

enum DecalFlags {
    DecalFlagsNone = 0x0,
    DecalFlagsFade = 0x1
};

enum LocationFlags {
    LocationNone = 0x0,
    LocationIndoors = 0x1,
    LocationBuildings = 0x2,
    LocationTerrain = 0x4
};

// bloodsplats are created at enemy death as locations of where blood decal needs to be applied
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
        this->blood_flags = DecalFlagsNone;
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
    int blood_flags;
    uint64_t fade_timer;
};

// store for all the bloodsplats to be applied
struct BloodsplatContainer {
    inline BloodsplatContainer() {
        uNumBloodsplats = 0;
    }

    virtual ~BloodsplatContainer() { }

    void AddBloodsplat(float x, float y, float z, float radius, unsigned char r, unsigned char g, unsigned char b);

    Bloodsplat pBloodsplats_to_apply[64];
    uint uNumBloodsplats;  // this loops round so old bloodsplats are replaced
};

// decal is the created geometry to display
struct Decal {
    void Decal_base_ctor();
    inline Decal() {
        Decal_base_ctor();
        decal_flags = DecalFlagsNone;
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
    int decal_flags;
};

// contains all of above
struct DecalBuilder {
    DecalBuilder() {
        this->log = EngineIoc::ResolveLogger();
        this->bloodsplat_container = EngineIoc::ResolveBloodsplatContainer();
        this->DecalsCount = 0;
    }

    virtual ~DecalBuilder() {}

    void AddBloodsplat(float x, float y, float z, float r, float g, float b, float radius);
    void Reset(bool bPreserveBloodsplats);
    char BuildAndApplyDecals(int light_level, char LocationFlags, struct stru154* FacePlane, int NumFaceVerts,
        RenderVertexSoft* FaceVerts, char ClipFlags, unsigned int uSectorID);
    bool Build_Decal_Geometry(
        int LightLevel, char LocationFlags, Bloodsplat* blood, float DecalRadius,
        unsigned int uColorMultiplier, float DecalDotDist, struct stru314* FacetNormals, signed int numfaceverts,
        RenderVertexSoft* faceverts, char uClipFlags);
    bool ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID);
    bool ApplyBloodSplat_OutdoorFace(ODMFace* pFace);
    bool ApplyBloodSplatToTerrain(struct Polygon* a2, Vec3_float_* a3, float* a4,
        struct RenderVertexSoft* a5, unsigned int uStripType, char a7);
    void DrawDecals(float z_bias);
    void DrawBloodsplats();
    void DrawDecalDebugOutlines();

    Decal Decals[1024];  // actual decal geom store
    unsigned int DecalsCount;  // number of decals

    // for building decal geom
    int uNumSplatsThisFace = 0;  // numeber of bloodsplats that overlap this face
    int WhichSplatsOnThisFace[1024]{};  // stores which ith element of blodsplats to apply outdoor bloodsplats/decals store for calc

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
