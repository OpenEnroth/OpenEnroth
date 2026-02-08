#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <span>

#include "Engine/mm7_data.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/SpawnPoint.h"

#include "Library/Geometry/Rect.h"

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

struct BLVDoor {
    DoorAttributes attributes;
    uint32_t doorId;
    Duration timeSinceTriggered;
    Vec3f direction; // Float direction vector
    int32_t moveLength;
    int32_t openSpeed; // In map units per real-time second.
    int32_t closeSpeed; // In map units per real-time second.
    int16_t *pVertexIDs;
    int16_t *pFaceIDs;
    int16_t *pSectorIDs;
    int16_t *pDeltaUs;
    int16_t *pDeltaVs;
    int16_t *pXOffsets;
    int16_t *pYOffsets;
    int16_t *pZOffsets;
    uint16_t numVertices;
    uint16_t numFaces;
    uint16_t numSectors;
    uint16_t numOffsets;
    DoorState state;
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
    GraphicsImage *GetTexture() const;

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

    inline bool IsAnimated() const {
        return this->uAttributes & FACE_ANIMATED;
    }
    inline void ToggleIsAnimated() {
        this->uAttributes = this->uAttributes & FACE_ANIMATED
                                ? this->uAttributes & ~FACE_ANIMATED
                                : this->uAttributes | FACE_ANIMATED;
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

    /** Array of indices into the vertex array for this face's vertices. Points into `IndoorLocation::pVertices` for
     * indoor faces, or `BSPModel::pVertices` for outdoor faces. Has `uNumVertices + 1` elements, where the last element
     * repeats the first vertex to close the polygon. */
    int16_t *pVertexIDs = nullptr;

    /** Array of U (horizontal) texture coordinates for each vertex, in texture pixels. Has `uNumVertices + 1` elements,
     * matching `pVertexIDs`. */
    int16_t *pVertexUs = nullptr;

    /** Array of V (vertical) texture coordinates for each vertex, in texture pixels. Has `uNumVertices + 1` elements,
     * matching `pVertexIDs`. */
    int16_t *pVertexVs = nullptr;

    uint16_t uFaceExtraID = 0;
    GraphicsImage *texture = nullptr; // Face texture, or nullptr if this face is animated.
    int animationId = 0; // Index into pTextureFrameTable for animated faces.
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

struct BLVSector {
    // Note that all spans below point into `IndoorLocation::sectorData` or `IndoorLocation::sectorLightData`.

    // TODO(captainurist): #enum
    int flags; // &8 checks floor level against portals, &0x10 adds additional node faces.
    std::span<uint16_t> floorIds; // Indices into `IndoorLocation::faces` for floor faces.
    std::span<uint16_t> wallIds; // Indices into `IndoorLocation::faces` for wall faces.
    std::span<uint16_t> ceilingIds; // Indices into `IndoorLocation::faces` for ceiling faces.
    std::span<uint16_t> portalIds; // Indices into `IndoorLocation::faces` for portal faces.
    std::span<uint16_t> nonBspFaceIds; // Subspan of `faceIds` for faces not in BSP tree, stored first in `faceIds`.
    std::span<uint16_t> faceIds; // Indices into `IndoorLocation::faces` for BSP traversal.
    std::span<uint16_t> decorationIds; // Indices into `pLevelDecorations`.
    std::span<uint16_t> lightIds; // Indices into `IndoorLocation::lights`.
    int16_t minAmbientLightLevel; // Minimum ambient light level, might actually be max ambient dim.
    int16_t firstBspNode; // Index into `IndoorLocation::nodes`, or -1 if none.
    BBoxf boundingBox; // Axis-aligned bounding box of this sector.
};

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

    void DrawIndoorFaces(bool bD3D);
    void PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, int uSectorID);

    std::string filename;
    unsigned int bLoaded = 0;
    std::vector<Vec3f> vertices;
    std::vector<BLVFace> faces;
    std::vector<BLVFaceExtra> faceExtras;
    std::vector<BLVSector> sectors;
    std::vector<BLVLight> lights;
    std::vector<BLVDoor> doors;
    std::vector<BSPNode> nodes;
    std::vector<BLVMapOutline> mapOutlines;
    std::vector<int16_t> faceData;
    std::vector<uint16_t> sectorData;
    std::vector<int16_t> doorsData;
    std::vector<uint16_t> sectorLightData;
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
    void Reset();

    // TODO(pskelton): move to party?
    int uPartySectorID = 0;
    int uPartyEyeSectorID = 0;

    unsigned int uTargetWidth = 0;
    unsigned int uTargetHeight = 0;
    Recti viewportRect;
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
 * @return                              Float Z coordinate of the floor/ceiling face for the given position.
 *                                      If wrong sector is supplied or actor is out of bounds, `-30000` is
 *                                      returned. If vertically transitioning between sectors '-29000' is returned.
 */
float BLV_GetFloorLevel(const Vec3f &pos, int uSectorID, int *pFaceID = nullptr);

/**
 * Initialises all door geometry into starting position on load.
 */
void BLV_InitialiseDoors();
/**
 * Updates all door geometry into current position and plays sounds if moving.
 * 
 * @offset 0x46F228
 */
void BLV_UpdateDoors();
/**
 * Updates the geomtry position of the supplied door. 
 * 
 * @param door                          Pointer to door to update.
 * @param distance                      Distance the door is displaced. 0 is open, door->uMoveLength when closed.
 */
void BLV_UpdateDoorGeometry(BLVDoor *door, int distance);

void BLV_UpdateActors();
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
