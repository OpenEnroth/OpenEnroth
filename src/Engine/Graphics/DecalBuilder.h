#pragma once

#include "Engine/IocContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/BSPModel.h"

#include "Utility/Flags.h"

using EngineIoc = Engine_::IocContainer;

enum class DecalFlag : int {
    DecalFlagsNone = 0x0,
    DecalFlagsFade = 0x1
};
using enum DecalFlag;
DECLARE_FLAGS(DecalFlags, DecalFlag)
DECLARE_OPERATORS_FOR_FLAGS(DecalFlags)

enum class LocationFlag {
    LocationNone = 0x0,
    LocationIndoors = 0x1,
    LocationBuildings = 0x2,
    LocationTerrain = 0x4
};
using enum LocationFlag;
DECLARE_FLAGS(LocationFlags, LocationFlag)
DECLARE_OPERATORS_FOR_FLAGS(LocationFlags)

// bloodsplats are created at enemy death as locations of where blood decal needs to be applied
struct Bloodsplat {
    float x = 0;
    float y = 0;
    float z = 0;
    float radius = 0;
    float dot_dist = 0;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    DecalFlags blood_flags = DecalFlagsNone;
    uint64_t fade_timer = 0;
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
    float Fade_by_time();

    virtual ~Decal() {}

    int uNumVertices;
    RenderVertexSoft pVertices[64];
    int16_t DecalXPos;
    int16_t DecalYPos;
    int16_t DecalZPos;
    uint32_t uColorMultiplier;
    int DimmingLevel;

    uint64_t fadetime;
    DecalFlags decal_flags;
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
    char BuildAndApplyDecals(int light_level, LocationFlags locationFlags, struct stru154* FacePlane, int NumFaceVerts,
                             RenderVertexSoft* FaceVerts, char ClipFlags, unsigned int uSectorID);
    bool Build_Decal_Geometry(
        int LightLevel, LocationFlags locationFlags, Bloodsplat* blood, float DecalRadius,
        unsigned int uColorMultiplier, float DecalDotDist, struct stru314* FacetNormals, signed int numfaceverts,
        RenderVertexSoft* faceverts, char uClipFlags);
    bool ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID);
    bool ApplyBloodSplat_OutdoorFace(ODMFace* pFace);
    bool ApplyBloodSplatToTerrain(struct Polygon* a2, Vec3f* a3, float* a4,
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
