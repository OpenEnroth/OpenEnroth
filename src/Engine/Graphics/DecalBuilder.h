#pragma once

#include <array>

#include "Engine/Graphics/RenderEntities.h"
#include "Engine/Time/Duration.h"
#include "Engine/Data/TileEnums.h"

#include "Utility/Flags.h"

struct ODMFace;
struct Planef;
struct stru314;

enum class DecalFlag : int {
    DecalFlagsNone = 0x0,
    DecalFlagsFade = 0x1
};
using enum DecalFlag;
MM_DECLARE_FLAGS(DecalFlags, DecalFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(DecalFlags)

enum class LocationFlag {
    LocationNone = 0x0,
    LocationIndoors = 0x1,
    LocationBuildings = 0x2,
    LocationTerrain = 0x4
};
using enum LocationFlag;
MM_DECLARE_FLAGS(LocationFlags, LocationFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(LocationFlags)

// bloodsplats are created at enemy death as locations of where blood decal needs to be applied
struct Bloodsplat {
    Vec3f pos; // Bloodsplat origin, usually 30 units above ground level where the monster was killed.
    float radius = 0;
    float faceDist = 0; // Signed distance from bloodsplat origin to the face plane (for the current face).
                        // TODO(captainurist): doesn't belong to this struct, should be moved out.
    Color color;
    DecalFlags blood_flags = DecalFlagsNone;
    Duration fade_timer;
};

// store for all the bloodsplats to be applied
struct BloodsplatContainer {
    void AddBloodsplat(const Vec3f &pos, float radius, Color color);

    std::array<Bloodsplat, 64> pBloodsplats_to_apply;
    unsigned int uNumBloodsplats = 0;  // this loops round so old bloodsplats are replaced
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
    std::array<RenderVertexSoft, 64> pVertices;
    int16_t DecalXPos;
    int16_t DecalYPos;
    int16_t DecalZPos;
    Color uColorMultiplier;
    int DimmingLevel;

    Duration fadetime;
    DecalFlags decal_flags;
};

// contains all of above
struct DecalBuilder {
    DecalBuilder();
    virtual ~DecalBuilder() {}

    void AddBloodsplat(const Vec3f &pos, Color color, float radius);
    void Reset(bool bPreserveBloodsplats);
    char BuildAndApplyDecals(int light_level, LocationFlags locationFlags, const Planef &FacePlane, int NumFaceVerts,
                             RenderVertexSoft *FaceVerts, char ClipFlags, int uSectorID);
    bool Build_Decal_Geometry(
        int LightLevel, LocationFlags locationFlags, Bloodsplat *blood, float DecalRadius,
        Color uColorMultiplier, float DecalDotDist, stru314 *FacetNormals, int numfaceverts,
        RenderVertexSoft *faceverts, char uClipFlags);
    bool ApplyBloodsplatDecals_IndoorFace(int uFaceID);
    bool ApplyBloodSplat_OutdoorFace(ODMFace *pFace);

    /**
     * @offset 0x0049BE8A
     * 
     * Attemps to apply a certain bloodsplat onto the supplied terrain triangle.
     * 
     * @param fading                        Whether blood splat should be fading.
     * @param terrnorm                      Normal vector of supplied triangle.
     * @param[out] tridotdist               Plane dot distance of supplied vertices.
     * @param triverts                      Vertices of terrain triangle to apply splat onto.
     * @param whichsplat                    Index of which bloodsplat in bloodsplat_container->pBloodsplats_to_apply[index] to use.
     * 
     * @return                              True if bloodsplat_container->uNumBloodsplats > 0, false otherwise.
     */
    bool ApplyBloodSplatToTerrain(bool fading, const Vec3f &terrnorm, float *tridotdist,
                                  RenderVertexSoft *triverts, const int whichsplat);
    void DrawDecals(float z_bias);
    void DrawBloodsplats();
    void DrawDecalDebugOutlines();

    std::array<Decal, 1024> Decals;  // actual decal geom store
    unsigned int DecalsCount;  // number of decals

    // for building decal geom
    int uNumSplatsThisFace = 0;  // numeber of bloodsplats that overlap this face
    std::array<int, 1024> WhichSplatsOnThisFace = {{}};  // stores which ith element of blodsplats to apply outdoor bloodsplats/decals store for calc

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
    BloodsplatContainer *bloodsplat_container;
};
