#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Engine/mm7_data.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/SpawnPoint.h"

#include "BSPModel.h"
#include "LocationInfo.h"
#include "LocationTime.h"
#include "LocationFunctions.h"
#include "FaceEnums.h"

struct BspRenderer;
struct IndoorLocation;
struct MapInfo;

struct BLVLight {
    Vec3f vPosition;
    int16_t uRadius = 0;
    char uRed = 0;
    char uGreen = 0;
    char uBlue = 0;
    char uType = 0;
    int16_t uAtributes = 0;  // & 0x08    doesn't light faces
    int16_t uBrightness = 0;
};

struct BLVDoor {  // 50h
    DoorAttributes uAttributes;
    uint32_t uDoorID;
    Duration uTimeSinceTriggered;
    Vec3f vDirection; // Float direction vector
    int32_t uMoveLength;
    int32_t uCloseSpeed; // In map units per real-time second.
    int32_t uOpenSpeed; // In map units per real-time second.
    int16_t *pVertexIDs;
    int16_t *pFaceIDs;
    int16_t *pSectorIDs;
    int16_t *pDeltaUs;
    int16_t *pDeltaVs;
    int16_t *pXOffsets;
    int16_t *pYOffsets;
    int16_t *pZOffsets;
    uint16_t uNumVertices;
    uint16_t uNumFaces;
    uint16_t uNumSectors;
    uint16_t uNumOffsets;
    DoorState uState;
};

struct BLVMapOutline {  // 0C
    uint16_t uVertex1ID;
    uint16_t uVertex2ID;
    uint16_t uFace1ID;
    uint16_t uFace2ID;
    int16_t sZ;
    uint16_t uFlags;
};

struct FlatFace {
    std::array<float, 104> u;
    std::array<float, 104> v;
};

/*   93 */
struct BLVFace {  // 60h
    void _get_normals(Vec3f *outU, Vec3f *outV);
    void FromODM(ODMFace *face);

    void SetTexture(std::string_view filename);
    GraphicsImage *GetTexture();

    inline bool Invisible() const {
        return uAttributes & FACE_IsInvisible;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool isPortal() const { return uAttributes & FACE_IsPortal; }
    inline bool isFluid() const { return uAttributes & FACE_IsFluid; }
    inline bool Indoor_sky() const {
        return uAttributes & FACE_INDOOR_SKY;
    }
    inline bool Clickable() const {
        return uAttributes & FACE_CLICKABLE;
    }
    inline bool Pressure_Plate() const {
        return uAttributes & FACE_PRESSURE_PLATE;
    }
    inline bool Ethereal() const { return uAttributes & FACE_ETHEREAL; }

    inline bool IsTextureFrameTable() const {
        return this->uAttributes & FACE_TEXTURE_FRAME;
    }
    inline void ToggleIsTextureFrameTable() {
        this->uAttributes = this->uAttributes & FACE_TEXTURE_FRAME
                                ? this->uAttributes & ~FACE_TEXTURE_FRAME
                                : this->uAttributes | FACE_TEXTURE_FRAME;
    }

    /**
     * @param[out] points               Coordinate storage. The storage is populated by the coordinates of this
     *                                  face's vertices projected onto this face's primary plane.
     * @param model_idx                 Model that this face belongs to, or `MODEL_INDOOR` for faces in indoor
     *                                  locations.
     * @param override_plane            Plane override.
     * @see BLVFace::Contains
     */
    void Flatten(FlatFace *points, int model_idx, FaceAttributes override_plane = 0) const;

    /**
     * @param pos                       Point to check.
     * @param model_idx                 Model that this face belongs to, or `MODEL_INDOOR` for faces in indoor
     *                                  locations.
     * @param slack                     If a point is at most `slack` units away from the edge, it'll still be
     *                                  considered to be lying on the edge.
     * @param override_plane            Plane override. By default the check is performed in the face's primary plane
     *                                  that is set in attributes, but this behavior can be overridden by e.g. passing
     *                                  `FACE_XY_PLANE`.
     * @return                          Whether the point lies inside this polygon, if projected on the face's
     *                                  primary plane.
     */
    bool Contains(const Vec3f &pos, int model_idx, int slack = 0, FaceAttributes override_plane = 0) const;

    Planef facePlane;
    PlaneZCalcf zCalc;
    FaceAttributes uAttributes;
    int16_t *pVertexIDs = nullptr;
    int16_t *pVertexUIDs = nullptr;
    int16_t *pVertexVIDs = nullptr;
    uint16_t uFaceExtraID = 0;
    void *resource = nullptr;  // int64_t or GraphicsImage
    int texunit = -1;
    int texlayer = -1;

    int uSectorID = 0;
    int uBackSectorID = 0;
    BBoxf pBounding;
    PolygonType uPolygonType = POLYGON_Invalid;
    uint8_t uNumVertices = 0;
};

struct BLVFaceExtra {
    bool HasEventHint();

    int face_id;
    uint16_t uAdditionalBitmapID; // TODO(captainurist): why is this one unused?
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    int16_t sCogNumber;
    uint16_t uEventID;
};

/*   95 */
struct BLVSector {  // 0x74
    int32_t field_0;  // flags?? &8 is for check floor level against portals &10 is for adding additonal node faces
    uint16_t uNumFloors;
    uint16_t *pFloors;
    uint16_t uNumWalls;
    uint16_t *pWalls;
    uint16_t uNumCeilings;
    uint16_t *pCeilings;
    uint16_t uNumFluids;
    uint16_t *pFluids;
    int16_t uNumPortals;
    uint16_t *pPortals;
    uint16_t uNumFaces;
    uint16_t uNumNonBSPFaces;
    uint16_t *pFaceIDs;
    uint16_t uNumCylinderFaces;
    int32_t pCylinderFaces;
    uint16_t uNumCogs;
    uint16_t *pCogs;
    uint16_t uNumDecorations;
    uint16_t *pDecorationIDs;
    uint16_t uNumMarkers;
    uint16_t *pMarkers;
    uint16_t uNumLights;
    uint16_t *pLights;
    int16_t uWaterLevel;
    int16_t uMistLevel;
    int16_t uLightDistanceMultiplier;
    int16_t uMinAmbientLightLevel;  // might be supposed to be max ambient dim actually
    int16_t uFirstBSPNode;
    int16_t exit_tag;
    BBoxf pBounding;
};

/*   89 */
struct IndoorLocation {
    //----- (00462592) --------------------------------------------------------
    inline IndoorLocation() {
        this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
        this->spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();
        this->particle_engine = EngineIocContainer::ResolveParticleEngine();
    }

    /**
     * @param sX                        X coordinate.
     * @param sY                        Y coordinate.
     * @param sZ                        Z coordinate.
     * @return                          Sector id at (X,Y,Z), or zero if (X,Y,Z) is outside the level bounds.
     */
    int GetSector(float sX, float sY, float sZ);
    // TODO(pskelton): do we need both still?
    int GetSector(const Vec3f &pos) {
        return GetSector(pos.x, pos.y, pos.z);
    }

    void Release();
    void Load(std::string_view filename, int num_days_played, int respawn_interval_days, bool *indoor_was_respawned);
    void Draw();

    /**
     * @offset 0x4488F7
     */
    void toggleLight(signed int uLightID, unsigned int bToggle);

    static unsigned int GetLocationIndex(std::string_view locationName);
    void DrawIndoorFaces(bool bD3D);
    void PrepareActorRenderList_BLV();
    void PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, int uSectorID);
    void PrepareItemsRenderList_BLV();

    std::string filename;
    unsigned int bLoaded = 0;
    std::vector<Vec3f> pVertices;
    std::vector<BLVFace> pFaces;
    std::vector<BLVFaceExtra> pFaceExtras;
    std::vector<BLVSector> pSectors;
    std::vector<BLVLight> pLights;
    std::vector<BLVDoor> pDoors;
    std::vector<BSPNode> pNodes;
    std::vector<BLVMapOutline> pMapOutlines;
    std::vector<int16_t> pLFaces;
    std::vector<uint16_t> ptr_0002B0_sector_rdata;
    std::vector<int16_t> ptr_0002B4_doors_ddata;
    std::vector<uint16_t> ptr_0002B8_sector_lrdata;
    std::vector<SpawnPoint> pSpawnPoints;
    LocationInfo dlv;
    LocationTime stru1;
    std::array<char, 875> _visible_outlines;
    char padding;

    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
};

extern IndoorLocation *pIndoor;

struct BLVRenderParams {
    inline BLVRenderParams() {
        uViewportX = 0;
        uViewportY = 0;
        uViewportZ = 0;
        uViewportW = 0;
    }

    void Reset();

    int uPartySectorID = 0;
    int uPartyEyeSectorID = 0;

    unsigned int uTargetWidth = 0;
    unsigned int uTargetHeight = 0;
    unsigned int uViewportX;
    unsigned int uViewportY;
    unsigned int uViewportZ;
    unsigned int uViewportW;
    int *pTargetZBuffer = nullptr;
    int uViewportHeight = 0;
    int uViewportWidth = 0;
    int uViewportCenterX = 0;
    int uViewportCenterY = 0;
    unsigned int uNumFacesRenderedThisFrame = 0;
};
extern BLVRenderParams *pBLVRenderParams;

char DoInteractionWithTopmostZObject(Pid pid);
// int sub_4AAEA6_transform(RenderVertexSoft *a1);
void BLV_UpdateUserInputAndOther();

/**
 * @param pos                           Actor's position.
 * @param uSectorID                     Actor's sector id.
 * @param[out] pFaceID                  Id of the closest floor/ceiling face for the provided position, or `-1`
 *                                      if wrong sector is supplied or actor is out of bounds. Pass `nullptr` to ignore.
 * @return                              Fixpoint Z coordinate of the floor/ceiling face for the given position.
 *                                      If wrong sector is supplied or actor is out of bounds, `-30000` is
 *                                      returned.
 */
float BLV_GetFloorLevel(const Vec3f &pos, int uSectorID, int *pFaceID = nullptr);
void BLV_UpdateDoors();
void UpdateActors_BLV();
void BLV_ProcessPartyActions();

/**
 * @offset 0x449A49
 */
void switchDoorAnimation(unsigned int uDoorID, DoorAction a2);
int CalcDistPointToLine(int a1, int a2, int a3, int a4, int a5, int a6);
void PrepareDrawLists_BLV();

/**
 * @offset 0x460A78
 */
void loadAndPrepareBLV(MapId mapid, bool bLoading);
int SpawnEncounterMonsters(MapInfo *a1, int a2);
int DropTreasureAt(ItemTreasureLevel trs_level, RandomItemType trs_type, Vec3f pos, uint16_t facing);
void SpawnRandomTreasure(MapInfo *mapInfo, SpawnPoint *a2);

void FindBillboardsLightLevels_BLV();

/**
 * @todo looks like this also works for ceilings, reflect in docs?
 *
 * @param pos                           Actor's position.
 * @param[in,out] pSectorID             Actor's cached sector id. If the cached sector id is no longer valid (e.g. an
 *                                      actor has already moved to another sector), then the new sector id is returned
 *                                      in this output parameter. If the actor moves out of level bounds (this happens),
 *                                      then this parameter is set to 0.
 * @param[out] pFaceID                  Id of the floor face on which the actor is standing, or `-1` if actor is outside
 *                                      the level boundaries. Pass `nullptr` to ignore.
 * @return                              Z coordinate for the floor at (X, Y), or `-30000` if actor is outside the
 *                                      level boundaries.
 */
float GetIndoorFloorZ(const Vec3f &pos, int *pSectorID, int *pFaceID = nullptr);

/**
 * @offset 0x0047272C.
 *
 * Same as `GetIndoorFloorZ`, but also tries jiggling the party around a bit if the collision point couldn't be
 * found.
 *
 * @see GetIndoorFloorZ
 */
float GetApproximateIndoorFloorZ(const Vec3f &pos, int *pSectorID, int *pFaceID = nullptr);

/**
 * @param target                         Vec3f of position to check line of sight to
 * @param from                           Vec3f of position to check line of sight from
 *
 * @return                              True if line of sight clear to target
 */
bool Check_LineOfSight(const Vec3f &target, const Vec3f &from);


/**
 * @param target                         Vec3f of position to check line of sight to
 * @param from                           Vec3f of position to check line of sight from
 *
 * @return                              True if line of sight obscurred by level geometery
 */
bool Check_LOS_Obscurred_Indoors(const Vec3f &target, const Vec3f &from);

/**
 * @param target                         Vec3f of position to check line of sight to
 * @param from                           Vec3f of position to check line of sight from
 *
 * @return                              True if line of sight obscurred by outdoor models
 */
bool Check_LOS_Obscurred_Outdoors_Bmodels(const Vec3f &target, const Vec3f &from);

extern BspRenderer *pBspRenderer;
