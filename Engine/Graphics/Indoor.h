#pragma once
#include <array>
#include <memory>
#include <string>

#include "Engine/mm7_data.h"
#include "Engine/IocContainer.h"

#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Camera.h"

struct IndoorLocation;

using EngineIoc = Engine_::IocContainer;

/*  358 */
#pragma pack(push, 1)
struct LightsData {
    int field_0;
    Plane_int_ plane_4;
    Vec3_int_ vec_14;
    Vec3_int_ vec_20;
    unsigned int uCurrentAmbientLightLevel;  // 0 to 31
    int field_30;
    int field_34;
    int field_38;
    int pDeltaUV[2];
    int field_44;
    int field_48;
    char field_4C[8];
    Vec3_int_ rotated_normal;
    Vec3_int_ vec_60;
    int field_6C;
    Vec3_int_ vec_70;
    int field_7C;
    Vec3_int_ vec_80;
    Vec3_int_ vec_8C;
    int field_98;
    Vec3_int_ vec_9C;
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
    int field_C_alert = 0;
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

/*  134 */
#pragma pack(push, 1)
struct stru141_actor_collision_object {
    /**
     * Prepares this struct by filling all necessary fields, and checks whether there is actually no movement.
     *
     * @param dt                        Time delta, in fixpoint seconds.
     * @return                          True if there is no movement, false otherwise.
     */
    bool PrepareAndCheckIfStationary(int dt);

    // actor is modeled as two spheres, basically "feet" & "head". Collisions are then done for both spheres.

    int check_hi;  // Check the hi sphere collisions. If not set, only the lo sphere is checked.
    int radius_lo;   // radius of the lo ("feet") sphere.
    int radius_hi;  // radius of the hi ("head") sphere.
    int height;  // actor height.
    int field_10;  // unused
    int field_14;  // unused
    int field_18;  // unused
    Vec3_int_ velocity;
    Vec3_int_ position_lo; // center of the lo sphere.
    Vec3_int_ position_hi; // center of the hi sphere.
    Vec3_int_ new_position_lo; // desired new position for the center of the lo sphere.
    Vec3_int_ new_position_hi; // desired new position for the center of the hi sphere.
    Vec3_int_ direction;  // movement direction, as a fixpoint unit vector.
    int speed = 0;
    int inv_speed;
    int move_distance;  // desired movement distance.
    int field_70;  // some dist modifier - blanked before coll links with adjusted_move_distance- slows/stops movement
    unsigned int uSectorID = 0;
    unsigned int pid;
    int adjusted_move_distance;  // movement distance after adjusting for collisions.
    int field_80;  // portal id??
    int field_84;  // pid of face
    int field_88;  // unsued
    BBox_int_ bbox = { 0, 0, 0, 0, 0, 0 };
    int field_A4;  // unused
};
#pragma pack(pop)
extern stru141_actor_collision_object collision_state;

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

/*  291 */
enum PolygonType : uint8_t {
    POLYGON_Invalid = 0x0,
    POLYGON_VerticalWall = 0x1,
    POLYGON_unk = 0x2,
    POLYGON_Floor = 0x3,
    POLYGON_InBetweenFloorAndWall = 0x4,
    POLYGON_Ceiling = 0x5,
    POLYGON_InBetweenCeilingAndWall = 0x6,
};

/*  147 */
#pragma pack(push, 1)

struct stru154 {
    //----- (0049B001) --------------------------------------------------------
    inline stru154() {}

    //----- (0049B027) --------------------------------------------------------
    inline ~stru154() {}

    void GetFacePlaneAndClassify(struct ODMFace *a2,
                                 struct BSPVertexBuffer *a3);
    void ClassifyPolygon(struct Vec3_float_ *pNormal, float dist);
    void GetFacePlane(struct ODMFace *pFace, struct BSPVertexBuffer *pVertices,
                      struct Vec3_float_ *pOutNormal, float *pOutDist);

    void (***vdestructor_ptr)(stru154 *, bool) = nullptr;
    Plane_float_ face_plane {};
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

/*   86 */
#pragma pack(push, 1)
struct SpawnPointMM6 {
    Vec3_int_ vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
};
#pragma pack(pop)

/*  102 */
#pragma pack(push, 1)
struct SpawnPointMM7 {
    //----- (00448DD6) --------------------------------------------------------
    SpawnPointMM7() {
        uRadius = 32;
        uAttributes = 0;
        uIndex = 0;
        uKind = 0;
        uGroup = 0;
    }

    Vec3_int_ vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
    unsigned int uGroup;

    inline bool IsMonsterSpawn() const { return uKind == 3; }
    inline bool IsTreasureSpawn() const { return uKind != 3; }
};
#pragma pack(pop)

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
    Vec3_short_ vPosition;
    int16_t uRadius;
    int16_t uAttributes;
    uint16_t uBrightness;
};
#pragma pack(pop)

/*   98 */
#pragma pack(push, 1)
struct BLVLightMM7 {  // 10h
    Vec3_short_ vPosition;
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

    unsigned int uAttributes;
    unsigned int uDoorID;
    unsigned int uTimeSinceTriggered;
    Vec3_int_ vDirection;
    int uMoveLength;
    int uOpenSpeed;
    int uCloseSpeed;
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
    int16_t field_48;
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

    void _get_normals(Vec3_int_ *a2, Vec3_int_ *a3);
    void FromODM(struct ODMFace *face);

    void SetTexture(const std::string &filename);
    Texture *GetTexture();

    bool Deserialize(struct BLVFace_MM7 *);

    inline bool Invisible() const {
        return (uAttributes & FACE_IsInvisible) != 0;
    }
    inline bool Visible() const { return !Invisible(); }
    inline bool Portal() const { return (uAttributes & FACE_IsPortal) != 0; } // TODO: rename IsPortal.
    inline bool Fluid() const { return (uAttributes & FACE_IsFluid) != 0; }
    inline bool Indoor_sky() const {
        return (uAttributes & FACE_INDOOR_SKY) != 0;
    }
    inline bool Clickable() const {
        return (uAttributes & FACE_CLICKABLE) != 0;
    }
    inline bool Pressure_Plate() const {
        return (uAttributes & FACE_PRESSURE_PLATE) != 0;
    }
    inline bool Ethereal() const { return (uAttributes & FACE_ETHEREAL) != 0; }

    inline bool IsTextureFrameTable() const {
        return this->uAttributes & FACE_TEXTURE_FRAME;
    }
    inline void ToggleIsTextureFrameTable() {
        this->uAttributes = this->uAttributes & FACE_TEXTURE_FRAME
                                ? this->uAttributes & ~FACE_TEXTURE_FRAME
                                : this->uAttributes | FACE_TEXTURE_FRAME;
    }

    /**
     * @param indoor                    Indoor location that this face belongs to.
     * @param x                         Point X coordinate.
     * @param y                         Point Y coordinate.
     * @return                          Whether the point at (X,Y) lies inside this polygon (if projected on XY plane).
     */
    bool ContainsXY(IndoorLocation *indoor, int x, int y) const;

    struct Plane_float_ pFacePlane {};
    struct Plane_int_ pFacePlane_old;
    int zCalc1;  // fixpoint a
    int zCalc2;  // fixpoint b
    int zCalc3;  // fixpoint c, plane = a*x + b*y + c.
    unsigned int uAttributes;
    uint16_t *pVertexIDs = nullptr;
    int16_t *pXInterceptDisplacements;
    int16_t *pYInterceptDisplacements;
    int16_t *pZInterceptDisplacements;
    int16_t *pVertexUIDs = nullptr;
    int16_t *pVertexVIDs = nullptr;
    uint16_t uFaceExtraID;
    void *resource;  // unsigned __int16  uBitmapID;
    uint16_t uSectorID;
    int16_t uBackSectorID;
    struct BBox_short_ pBounding {};
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
    BBox_short_ pBounding;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BLVMapOutlines {
    int uNumOutlines;
    BLVMapOutline pOutlines[7000];
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

        bLoaded = 0;
        ptr_0002B8_sector_lrdata = 0;
        ptr_0002B4_doors_ddata = 0;
        ptr_0002B0_sector_rdata = 0;
        pLFaces = 0;
        pVertices = 0;
        pFaces = 0;
        pFaceExtras = 0;
        pSectors = 0;
        pLights = 0;
        pDoors = 0;
        pNodes = 0;
        pMapOutlines = 0;
        uNumSpawnPoints = 0;
        pSpawnPoints = 0;
        uNumSectors = 0;
    }

    /**
     * @param sX                        X coordinate.
     * @param sY                        Y coordinate.
     * @param sZ                        Z coordinate.
     * @return                          Sector id at (X,Y,Z), or zero if (X,Y,Z) is outside the level bounds.
     */
    int GetSector(int sX, int sY, int sZ);

    int GetSector(const Vec3_int_ &pos) {
        return GetSector(pos.x, pos.y, pos.z);
    }

    int GetSector(const Vec3_short_ &pos) {
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
    unsigned int bLoaded;
    char field_54[404];
    struct BLVHeader blv;
    unsigned int uNumVertices;
    Vec3_short_ *pVertices;
    unsigned int uNumFaces;
    struct BLVFace *pFaces;
    unsigned int uNumFaceExtras;
    struct BLVFaceExtra *pFaceExtras;
    int uNumSectors;
    struct BLVSector *pSectors;
    int uNumLights;
    struct BLVLightMM7 *pLights;
    int uNumDoors;
    struct BLVDoor *pDoors;
    unsigned int uNumNodes;
    struct BSPNode *pNodes;
    BLVMapOutlines *pMapOutlines;
    uint16_t *pLFaces;
    uint16_t *ptr_0002B0_sector_rdata;
    uint16_t *ptr_0002B4_doors_ddata;
    uint16_t *ptr_0002B8_sector_lrdata;
    unsigned int uNumSpawnPoints;
    struct SpawnPointMM7 *pSpawnPoints;
    struct DDM_DLV_Header dlv;
    LocationTime_stru1 stru1;
    char _visible_outlines[875];
    char padding;

    Log *log = nullptr;
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

void PrepareBspRenderList_BLV();
void AddBspNodeToRenderList(unsigned int node_id);
void AddNodeBSPFaces(unsigned int node_id, unsigned int uFirstNode);  // idb
char DoInteractionWithTopmostZObject(int pid);
// int sub_4AAEA6_transform(struct RenderVertexSoft *a1);
unsigned int FaceFlowTextureOffset(unsigned int uFaceID);  // idb
void BLV_UpdateUserInputAndOther();

/**
 * @param x                             Actor's fixpoint X position.
 * @param y                             Actor's fixpoint Y position.
 * @param z                             Actor's fixpoint Z position.
 * @param uSectorID                     Actor's sector id.
 * @param[out] pFaceID                  Id of the closest floor/ceiling face for the provided position.
 * @return                              Fixpoint Z coordinate of the floor/ceiling face for the given position.
 *                                      If wrong sector is supplied, `-30000` is returned.
 */
int BLV_GetFloorLevel(int x, int y, int z, unsigned int uSectorID, unsigned int *pFaceID);
void BLV_UpdateDoors();
void UpdateActors_BLV();
void BLV_ProcessPartyActions();
void Door_switch_animation(unsigned int uDoorID, int a2);  // idb: sub_449A49
int CalcDistPointToLine(int a1, int a2, int a3, int a4, int a5, int a6);
void PrepareDrawLists_BLV();
void PrepareToLoadBLV(unsigned int bLoading);
int GetAlertStatus();
int SpawnEncounterMonsters(struct MapInfo *a1, int a2);
int DropTreasureAt(int trs_level, signed int trs_type, int x, int y, int z, uint16_t facing);

bool PointInPolyIndoor(int x, int y, int z, struct BLVFace *face);
bool PointInPolyOutdoor(int a1, int a2, int a3, struct ODMFace *face,
                struct BSPVertexBuffer *a5);


/*  164 */
#pragma pack(push, 1)
struct BspRenderer_ViewportNode {
    //----- (0043F2BF) --------------------------------------------------------
    inline BspRenderer_ViewportNode() {}

    //----- (0043F2A9) --------------------------------------------------------
    ~BspRenderer_ViewportNode() {}

    uint16_t uSectorID = 0;  // sector that this node shows
    uint16_t uFaceID;
    unsigned int viewing_portal_id;  // portal/ node through which we're seeing this node
    IndoorCameraD3D_Vec4 ViewportNodeFrustum[4];  // frustum planes of portal
    RenderVertexSoft pPortalBounding[4];  // extents of portal
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BspFace {
    uint16_t uFaceID;
    uint16_t uNodeID;
};
#pragma pack(pop)

/*  163 */
#pragma pack(push, 1)
struct BspRenderer {  // stru170
    //----- (0043F282) --------------------------------------------------------
    inline BspRenderer() {
        num_faces = 0;
        num_nodes = 0;
        uNumVisibleNotEmptySectors = 0;
    }

    void AddFaceToRenderList_d3d(unsigned int node_id, unsigned int uFaceID);
    void MakeVisibleSectorList();

    unsigned int num_faces;
    BspFace faces[1000]{};

    unsigned int num_nodes;
    BspRenderer_ViewportNode nodes[150];

    unsigned int uNumVisibleNotEmptySectors;
    uint16_t pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[6]{};
};
#pragma pack(pop)

void FindBillboardsLightLevels_BLV();

/**
 * Same as `collide_against_floor`, but also tries jiggling the party around a bit if the collision point couldn't be
 * found.
 *
 * @see collide_against_floor
 */
int collide_against_floor_approximate(int x, int y, int z, unsigned int *pSectorID, unsigned int *pFaceID);

bool Check_LineOfSight(int to_x, int to_y, int to_z, Vec3_int_ from);

extern struct BspRenderer* pBspRenderer;
