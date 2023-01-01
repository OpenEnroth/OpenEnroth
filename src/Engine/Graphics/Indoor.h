#pragma once
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Engine/mm7_data.h"
#include "Engine/IocContainer.h"
#include "Engine/SpawnPoint.h"

#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/IRender.h"

struct IndoorLocation;

using EngineIoc = Engine_::IocContainer;

/*  358 */
#pragma pack(push, 1)
struct LightsData {
    int field_0;
    Planei plane_4;
    Vec3i vec_14;
    Vec3i vec_20;
    unsigned int uCurrentAmbientLightLevel;  // 0 to 31
    int field_30;
    int field_34;
    int field_38;
    int pDeltaUV[2];
    int field_44;
    int field_48;
    char field_4C[8];
    Vec3i rotated_normal;
    Vec3i vec_60;
    int field_6C;
    Vec3i vec_70;
    int field_7C;
    Vec3i vec_80;
    Vec3i vec_8C;
    int field_98;
    Vec3i vec_9C;
    int field_A8;
    unsigned int uNumLightsApplied;
    int _blv_lights_radii[20];
    int _blv_lights_inv_radii[20];
    int _blv_lights_xs[20];
    int _blv_lights_ys[20];
    int _blv_lights_light_dot_faces[20];
    int field_240;
    int field_244;
    int field_248;
    int field_24C;
    int field_250;
    int field_254;
    int field_258;
    int field_25C;
    int field_260;
    int field_264;
    int field_268;
    int field_26C;
    int field_270;
    int field_274;
    int field_278;
    int field_27C;
    int field_280;
    int field_284;
    int field_288;
    int field_28C;
    int _blv_lights_zs[20];
    float _blv_lights_rs[20];
    float _blv_lights_gs[20];
    float _blv_lights_bs[20];
    char _blv_lights_types[20];
    int field_3E4;
    int field_3E8;
    int field_3EC;
    int field_3F0;
    int field_3F4;
    unsigned int uDefaultAmbientLightLevel;
};
#pragma pack(pop)

extern LightsData Lights;  // idb

#pragma pack(push, 1)
struct DDM_DLV_Header {
    //----- (00462607) --------------------------------------------------------
    inline DDM_DLV_Header() {}

    int uNumRespawns = 0;
    int uLastRepawnDay = 0;
    int uReputation = 0;
    int field_C_alert = 0; // Actually bool
    unsigned int uNumFacesInBModels = 0;
    unsigned int uNumDecorations = 0;
    unsigned int uNumBModels = 0;
    int field_1C = 0;
    int field_20 = 0;
    int field_24 = 0;
};
#pragma pack(pop)

/*  345 */
#pragma pack(push, 1)
struct stru315 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    uint16_t *field_34_palette;
    uint16_t *pTextureLOD;
    unsigned int *pDepthBuffer;
    uint16_t *pColorBuffer;
};
#pragma pack(pop)

/*  346 */
#pragma pack(push, 1)
struct stru316 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    uint16_t *field_24_palette;
};
#pragma pack(pop)

/*  378 */
#pragma pack(push, 1)
struct stru337_stru0 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
};
#pragma pack(pop)

/*  377 */
#pragma pack(push, 1)
struct stru337_unused {
    stru337_stru0 field_0;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    stru337_stru0 field_34;
};
#pragma pack(pop)
extern stru337_unused _DLV_header_unused;

/*  147 */
#pragma pack(push, 1)

struct stru154 {
    //----- (0049B001) --------------------------------------------------------
    inline stru154() {}

    //----- (0049B027) --------------------------------------------------------
    inline ~stru154() {}

    void GetFacePlaneAndClassify(ODMFace *a2,
                                 const std::vector<Vec3i> &a3);
    void ClassifyPolygon(Vec3f *pNormal, float dist);
    void GetFacePlane(ODMFace *pFace, const std::vector<Vec3i> &pVertices,
                      Vec3f *pOutNormal, float *pOutDist);

    Planef face_plane {};
    PolygonType polygonType {};
    char field_15 = 0;
    char field_16 = 0;
    char field_17 = 0;
};
#pragma pack(pop)

/*  392 */
#pragma pack(push, 1)
struct stru352 {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
};
#pragma pack(pop)
// extern std::array<stru352, 480> stru_F83B80;

#pragma pack(push, 1)
struct LocationTime_stru1 {
    GameTime last_visit {};
    char sky_texture_name[12] {};
    int day_attrib = 0;
    int day_fogrange_1 = 0;
    int day_fogrange_2 = 0;
    char field_2F4[24] {};
};
#pragma pack(pop)

/*  319 */
enum LEVEL_TYPE {
    LEVEL_null = 0,
    LEVEL_Indoor = 0x1,
    LEVEL_Outdoor = 0x2,
};
extern LEVEL_TYPE uCurrentlyLoadedLevelType;

/*   90 */
#pragma pack(push, 1)
struct BLVHeader {
    char field_0[104];
    unsigned int uFaces_fdata_Size;
    unsigned int uSector_rdata_Size;
    unsigned int uSector_lrdata_Size;
    unsigned int uDoors_ddata_Size;
    char field_78[16];
};
#pragma pack(pop)

/*   96 */
#pragma pack(push, 1)
struct BLVSectorMM8 {
    int dword_000074;
};
#pragma pack(pop)

/*   97 */
#pragma pack(push, 1)
struct BLVLightMM6 {
    Vec3s vPosition;
    int16_t uRadius;
    int16_t uAttributes;
    uint16_t uBrightness;
};
#pragma pack(pop)

/*   98 */
#pragma pack(push, 1)
struct BLVLightMM7 {  // 10h
    Vec3s vPosition;
    int16_t uRadius;
    char uRed;
    char uGreen;
    char uBlue;
    char uType;
    int16_t uAtributes;  // & 0x08    doesn't light faces
    int16_t uBrightness;
};
#pragma pack(pop)

/*   99 */
#pragma pack(push, 1)
struct BLVLightMM8 {
    int uID;
};
#pragma pack(pop)

/*  100 */
#pragma pack(push, 1)
struct BLVDoor {  // 50h
    enum State : uint16_t {
        Closed = 0,
        Opening = 1,
        Open = 2,
        Closing = 3
    };

    DoorAttributes uAttributes;
    uint32_t uDoorID;
    uint32_t uTimeSinceTriggered;
    Vec3i vDirection;
    int32_t uMoveLength;
    int32_t uOpenSpeed;
    int32_t uCloseSpeed;
    uint16_t *pVertexIDs;
    uint16_t *pFaceIDs;
    uint16_t *pSectorIDs;
    int16_t *pDeltaUs;
    int16_t *pDeltaVs;
    uint16_t *pXOffsets;
    uint16_t *pYOffsets;
    uint16_t *pZOffsets;
    uint16_t uNumVertices;
    uint16_t uNumFaces;
    uint16_t uNumSectors;
    uint16_t uNumOffsets;
    State uState;
    int16_t field_4E;
};
#pragma pack(pop)

/*  101 */
#pragma pack(push, 1)
struct BLVMapOutline {  // 0C
    uint16_t uVertex1ID;
    uint16_t uVertex2ID;
    uint16_t uFace1ID;
    uint16_t uFace2ID;
    int16_t sZ;
    uint16_t uFlags;
};
#pragma pack(pop)

struct FlatFace {
    std::array<int32_t, 104> u;
    std::array<int32_t, 104> v;
};

enum {
    MODEL_INDOOR = -1
};

/*   93 */
#pragma pack(push, 1)
struct BLVFace {  // 60h
    //----- (0046ED02) --------------------------------------------------------
    inline BLVFace() {
        this->uNumVertices = 0;
        this->uAttributes = 0;
        this->uFaceExtraID = 0;
        this->pVertexIDs = nullptr;
        this->pZInterceptDisplacements = nullptr;
        this->pYInterceptDisplacements = nullptr;
        this->pXInterceptDisplacements = nullptr;
    }

    void _get_normals(Vec3i *a2, Vec3i *a3);
    void FromODM(struct ODMFace *face);

    void SetTexture(const std::string &filename);
    Texture *GetTexture();

    inline bool Invisible() const {
        return uAttributes & FACE_IsInvisible;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return uAttributes & FACE_IsPortal; } // TODO: rename IsPortal.
    inline bool Fluid() const { return uAttributes & FACE_IsFluid; }
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
    bool Contains(const Vec3i &pos, int model_idx, int slack = 0, FaceAttributes override_plane = 0) const;

    struct Planef pFacePlane;
    struct Planei pFacePlane_old;
    PlaneZCalcll zCalc;
    FaceAttributes uAttributes;
    uint16_t *pVertexIDs = nullptr;
    int16_t *pXInterceptDisplacements;
    int16_t *pYInterceptDisplacements;
    int16_t *pZInterceptDisplacements;
    int16_t *pVertexUIDs = nullptr;
    int16_t *pVertexVIDs = nullptr;
    uint16_t uFaceExtraID;
    void *resource;  // uint16_t  uBitmapID;
    int texunit = -1;
    int texlayer = -1;

    uint16_t uSectorID;
    int16_t uBackSectorID;
    BBoxs pBounding;
    PolygonType uPolygonType;
    uint8_t uNumVertices;
    char field_5E = 0;
    char field_5F = 0;
};
#pragma pack(pop)

/*   94 */
#pragma pack(push, 1)
struct BLVFaceExtra {  // 24h
    bool HasEventHint();

    int16_t field_0;
    int16_t field_2;
    int16_t field_4;
    int16_t field_6;
    int16_t field_8;
    int16_t field_A;
    int16_t face_id;
    uint16_t uAdditionalBitmapID;
    int16_t field_10;
    int16_t field_12;
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    int16_t sCogNumber;
    uint16_t uEventID;
    int16_t field_1C;
    int16_t field_1E;
    int16_t field_20;
    int16_t field_22;
};
#pragma pack(pop)

/*   95 */
#pragma pack(push, 1)
struct BLVSector {  // 0x74
    int32_t field_0;  // flags?? &8 is for check floor level against portals &10 is for adding additonal node faces
    uint16_t uNumFloors;
    int16_t field_6;
    uint16_t *pFloors;
    uint16_t uNumWalls;
    int16_t field_E;
    uint16_t *pWalls;
    uint16_t uNumCeilings;
    int16_t field_16;
    uint16_t *pCeilings;
    uint16_t uNumFluids;
    int16_t field_1E;
    uint16_t *pFluids;
    int16_t uNumPortals;
    int16_t field_26;
    uint16_t *pPortals;
    uint16_t uNumFaces;
    uint16_t uNumNonBSPFaces;
    uint16_t *pFaceIDs;
    uint16_t uNumCylinderFaces;
    int16_t field_36;
    int32_t pCylinderFaces;
    uint16_t uNumCogs;
    int16_t field_3E;
    uint16_t *pCogs;
    uint16_t uNumDecorations;
    int16_t field_46;
    uint16_t *pDecorationIDs;
    uint16_t uNumMarkers;
    int16_t field_4E;
    uint16_t *pMarkers;
    uint16_t uNumLights;
    int16_t field_56;
    uint16_t *pLights;
    int16_t uWaterLevel;
    int16_t uMistLevel;
    int16_t uLightDistanceMultiplier;
    int16_t uMinAmbientLightLevel;  // might be supposed to be max ambient dim actually
    int16_t uFirstBSPNode;
    int16_t exit_tag;
    BBoxs pBounding;
};
#pragma pack(pop)

/*   89 */
#pragma pack(push, 1)
struct IndoorLocation {
    //----- (00462592) --------------------------------------------------------
    inline IndoorLocation() {
        this->log = EngineIoc::ResolveLogger();
        this->decal_builder = EngineIoc::ResolveDecalBuilder();
        this->spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();
        this->lightmap_builder = EngineIoc::ResolveLightmapBuilder();
        this->particle_engine = EngineIoc::ResolveParticleEngine();
    }

    /**
     * @param sX                        X coordinate.
     * @param sY                        Y coordinate.
     * @param sZ                        Z coordinate.
     * @return                          Sector id at (X,Y,Z), or zero if (X,Y,Z) is outside the level bounds.
     */
    int GetSector(int sX, int sY, int sZ);

    int GetSector(const Vec3i &pos) {
        return GetSector(pos.x, pos.y, pos.z);
    }

    int GetSector(const Vec3s &pos) {
        return GetSector(pos.x, pos.y, pos.z);
    }

    void Release();
    bool Alloc();
    bool Load(const std::string &filename, int num_days_played,
              int respawn_interval_days, char *pDest);
    void Draw();
    void ToggleLight(signed int uLightID, unsigned int bToggle);

    static unsigned int GetLocationIndex(const char *Str1);
    void DrawIndoorFaces(bool bD3D);
    // static void ExecDraw_sw(unsigned int uFaceID);
    void ExecDraw_d3d(unsigned int uFaceID,
                             struct IndoorCameraD3D_Vec4 *pVertices,
                             unsigned int uNumVertices,
                             struct RenderVertexSoft *pPortalBounding);
    void PrepareActorRenderList_BLV();
    void PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, unsigned int uSectorID);
    void PrepareItemsRenderList_BLV();

    std::string filename;
    char field_20[48];
    unsigned int bLoaded = 0;
    char field_54[404];
    BLVHeader blv;
    std::vector<Vec3s> pVertices;
    std::vector<BLVFace> pFaces;
    std::vector<BLVFaceExtra> pFaceExtras;
    std::vector<BLVSector> pSectors;
    std::vector<BLVLightMM7> pLights;
    std::vector<BLVDoor> pDoors;
    std::vector<BSPNode> pNodes;
    std::vector<BLVMapOutline> pMapOutlines;
    std::vector<uint16_t> pLFaces;
    std::vector<uint16_t> ptr_0002B0_sector_rdata;
    std::vector<uint16_t> ptr_0002B4_doors_ddata;
    std::vector<uint16_t> ptr_0002B8_sector_lrdata;
    std::vector<SpawnPoint> pSpawnPoints;
    DDM_DLV_Header dlv;
    LocationTime_stru1 stru1;
    std::array<char, 875> _visible_outlines;
    char padding;

    Logger *log = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    LightmapBuilder *lightmap_builder = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
};
#pragma pack(pop)

extern IndoorLocation *pIndoor;

/*  162 */
#pragma pack(push, 1)
struct BLVRenderParams {
    inline BLVRenderParams() {
        uViewportX = 0;
        uViewportY = 0;
        uViewportZ = 0;
        uViewportW = 0;
    }

    void Reset();

    int field_0_timer_ = 0;
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
#pragma pack(pop)
extern BLVRenderParams *pBLVRenderParams;

char DoInteractionWithTopmostZObject(int pid);
// int sub_4AAEA6_transform(struct RenderVertexSoft *a1);
unsigned int FaceFlowTextureOffset(unsigned int uFaceID);  // idb
void BLV_UpdateUserInputAndOther();

/**
 * @param pos                           Actor's position.
 * @param uSectorID                     Actor's sector id.
 * @param[out] pFaceID                  Id of the closest floor/ceiling face for the provided position, or `-1`
 *                                      if wrong sector is supplied or actor is out of bounds.
 * @return                              Fixpoint Z coordinate of the floor/ceiling face for the given position.
 *                                      If wrong sector is supplied or actor is out of bounds, `-30000` is
 *                                      returned.
 */
int BLV_GetFloorLevel(const Vec3i &pos, unsigned int uSectorID, unsigned int *pFaceID);
void BLV_UpdateDoors();
void UpdateActors_BLV();
void BLV_ProcessPartyActions();
void Door_switch_animation(unsigned int uDoorID, int a2);  // idb: sub_449A49
int CalcDistPointToLine(int a1, int a2, int a3, int a4, int a5, int a6);
void PrepareDrawLists_BLV();
void PrepareToLoadBLV(bool bLoading);
bool GetAlertStatus();
int SpawnEncounterMonsters(struct MapInfo *a1, int a2);
int DropTreasureAt(ITEM_TREASURE_LEVEL trs_level, signed int trs_type, int x, int y, int z, uint16_t facing);

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
 *                                      the level boundaries.
 * @return                              Z coordinate for the floor at (X, Y), or `-30000` if actor is outside the
 *                                      level boundaries.
 */
int GetIndoorFloorZ(const Vec3i &pos, unsigned int *pSectorID, unsigned int *pFaceID);

/**
 * @offset 0x0047272C.
 *
 * Same as `GetIndoorFloorZ`, but also tries jiggling the party around a bit if the collision point couldn't be
 * found.
 *
 * @see GetIndoorFloorZ
 */
int GetApproximateIndoorFloorZ(const Vec3i &pos, unsigned int *pSectorID, unsigned int *pFaceID);

bool Check_LineOfSight(int to_x, int to_y, int to_z, Vec3i from);


/**
 * @param from                           Vec3i of position to check line of sight from
 * @param to                             Vec3i of position to check line of sight to
 *
 * @return                              True if line of sight obscurred by level geometery
 */
bool Check_LOS_Obscurred_Indoors(const Vec3i &from, const Vec3i &to);

extern struct BspRenderer* pBspRenderer;
